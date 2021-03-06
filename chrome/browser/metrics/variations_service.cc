// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/metrics/variations_service.h"

#include "base/base64.h"
#include "base/build_time.h"
#include "base/memory/scoped_ptr.h"
#include "base/metrics/field_trial.h"
#include "base/version.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/metrics/proto/trials_seed.pb.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/common/pref_names.h"
#include "content/public/common/url_fetcher.h"
#include "googleurl/src/gurl.h"
#include "net/base/load_flags.h"
#include "net/base/network_change_notifier.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request_status.h"

namespace {

// Default server of Variations seed info.
const char kDefaultVariationsServer[] =
    "https://clients4.google.com/chrome-variations/seed";
const int kMaxRetrySeedFetch = 5;

// Maps chrome_variations::Study_Channel enum values to corresponding
// chrome::VersionInfo::Channel enum values.
chrome::VersionInfo::Channel ConvertStudyChannelToVersionChannel(
    chrome_variations::Study_Channel study_channel) {
  switch (study_channel) {
    case chrome_variations::Study_Channel_CANARY:
      return chrome::VersionInfo::CHANNEL_CANARY;
    case chrome_variations::Study_Channel_DEV:
      return chrome::VersionInfo::CHANNEL_DEV;
    case chrome_variations::Study_Channel_BETA:
      return chrome::VersionInfo::CHANNEL_BETA;
    case chrome_variations::Study_Channel_STABLE:
      return chrome::VersionInfo::CHANNEL_STABLE;
  }
  // All enum values of |study_channel| were handled above.
  NOTREACHED();
  return chrome::VersionInfo::CHANNEL_UNKNOWN;
}

// Converts |date_time| in chrome_variations::Study date format to base::Time.
base::Time ConvertStudyDateToBaseTime(int64 date_time) {
  return base::Time::UnixEpoch() + base::TimeDelta::FromSeconds(date_time);
}

}  // namespace

VariationsService::VariationsService() {}
VariationsService::~VariationsService() {}

bool VariationsService::CreateTrialsFromSeed(PrefService* local_prefs) {
  chrome_variations::TrialsSeed seed;
  if (!LoadTrialsSeedFromPref(local_prefs, &seed))
    return false;

  const int64 date_value = local_prefs->GetInt64(prefs::kVariationsSeedDate);
  const base::Time seed_date = base::Time::FromInternalValue(date_value);
  const base::Time build_time = base::GetBuildTime();
  // Use the build time for date checks if either the seed date is invalid or
  // the build time is newer than the seed date.
  base::Time reference_date = seed_date;
  if (seed_date.is_null() || seed_date < build_time)
    reference_date = build_time;

  const chrome::VersionInfo current_version_info;
  if (!current_version_info.is_valid())
    return false;

  for (int i = 0; i < seed.study_size(); ++i) {
    if (ShouldAddStudy(seed.study(i), current_version_info, reference_date))
      CreateTrialFromStudy(seed.study(i), reference_date);
  }

  return true;
}

void VariationsService::StartFetchingVariationsSeed() {
  if (net::NetworkChangeNotifier::IsOffline())
    return;

  pending_seed_request_.reset(content::URLFetcher::Create(
      GURL(kDefaultVariationsServer), net::URLFetcher::GET, this));
  pending_seed_request_->SetLoadFlags(net::LOAD_DO_NOT_SEND_COOKIES |
                                      net::LOAD_DO_NOT_SAVE_COOKIES);
  pending_seed_request_->SetRequestContext(
      g_browser_process->system_request_context());
  pending_seed_request_->SetMaxRetries(kMaxRetrySeedFetch);
  pending_seed_request_->Start();
}

void VariationsService::OnURLFetchComplete(const net::URLFetcher* source) {
  DCHECK_EQ(pending_seed_request_.get(), source);
  // When we're done handling the request, the fetcher will be deleted.
  scoped_ptr<const net::URLFetcher> request(
      pending_seed_request_.release());
  if (request->GetStatus().status() != net::URLRequestStatus::SUCCESS ||
      request->GetResponseCode() != 200)
    return;

  std::string seed_data;
  bool success = request->GetResponseAsString(&seed_data);
  DCHECK(success);

  base::Time response_date;
  success = request->GetResponseHeaders()->GetDateValue(&response_date);
  DCHECK(success || response_date.is_null());

  StoreSeedData(seed_data, response_date, g_browser_process->local_state());
}

// static
void VariationsService::RegisterPrefs(PrefService* prefs) {
  prefs->RegisterStringPref(prefs::kVariationsSeed, std::string());
  prefs->RegisterInt64Pref(prefs::kVariationsSeedDate,
                           base::Time().ToInternalValue());
}

void VariationsService::StoreSeedData(const std::string& seed_data,
                                      const base::Time& seed_date,
                                      PrefService* local_prefs) {
  // Only store the seed data if it parses correctly.
  chrome_variations::TrialsSeed seed;
  if (!seed.ParseFromString(seed_data)) {
    VLOG(1) << "Variations Seed data from server is not in valid proto format, "
            << "rejecting the seed.";
    return;
  }

  std::string base64_seed_data;
  if (!base::Base64Encode(seed_data, &base64_seed_data)) {
    VLOG(1) << "Variations Seed data from server fails Base64Encode, rejecting "
            << "the seed.";
    return;
  }

  local_prefs->SetString(prefs::kVariationsSeed, base64_seed_data);
  local_prefs->SetInt64(prefs::kVariationsSeedDate,
                        seed_date.ToInternalValue());
}

// static
bool VariationsService::ShouldAddStudy(
    const chrome_variations::Study& study,
    const chrome::VersionInfo& version_info,
    const base::Time& reference_date) {
  if (!CheckStudyChannel(study, chrome::VersionInfo::GetChannel())) {
    DVLOG(1) << "Filtered out study " << study.name() << " due to version.";
    return false;
  }

  if (!CheckStudyVersion(study, version_info.Version())) {
    DVLOG(1) << "Filtered out study " << study.name() << " due to version.";
    return false;
  }

  if (!CheckStudyStartDate(study, reference_date)) {
    DVLOG(1) << "Filtered out study " << study.name() << " due to start date.";
    return false;
  }

  DVLOG(1) << "Kept study " << study.name() << ".";
  return true;
}

// static
bool VariationsService::CheckStudyChannel(
    const chrome_variations::Study& study,
    chrome::VersionInfo::Channel channel) {
  // An empty channel list matches all channels.
  if (study.channel_size() == 0)
    return true;

  for (int i = 0; i < study.channel_size(); ++i) {
    if (ConvertStudyChannelToVersionChannel(study.channel(i)) == channel)
      return true;
  }
  return false;
}

// static
bool VariationsService::CheckStudyVersion(const chrome_variations::Study& study,
                                          const std::string& version_string) {
  const Version version(version_string);
  if (!version.IsValid()) {
    NOTREACHED();
    return false;
  }

  if (study.has_min_version()) {
    const Version min_version(study.min_version());
    if (!min_version.IsValid())
      return false;
    if (version.CompareTo(min_version) < 0)
      return false;
  }

  if (study.has_max_version()) {
    const Version max_version(study.max_version());
    if (!max_version.IsValid())
      return false;
    if (version.CompareTo(max_version) > 0)
      return false;
  }

  return true;
}

// static
bool VariationsService::CheckStudyStartDate(
    const chrome_variations::Study& study,
    const base::Time& date_time) {
  if (study.has_start_date()) {
    const base::Time start_date =
        ConvertStudyDateToBaseTime(study.start_date());
    return date_time >= start_date;
  }

  return true;
}

bool VariationsService::IsStudyExpired(const chrome_variations::Study& study,
                                       const base::Time& date_time) {
  if (study.has_expiry_date()) {
    const base::Time expiry_date =
        ConvertStudyDateToBaseTime(study.expiry_date());
    return date_time >= expiry_date;
  }

  return false;
}

// static
bool VariationsService::ValidateStudyAndComputeTotalProbability(
    const chrome_variations::Study& study,
    base::FieldTrial::Probability* total_probability) {
  // At the moment, a missing default_experiment_name makes the study invalid.
  if (study.default_experiment_name().empty()) {
    DVLOG(1) << study.name() << " has no default experiment defined.";
    return false;
  }

  const std::string& default_group_name = study.default_experiment_name();
  base::FieldTrial::Probability divisor = 0;

  bool found_default_group = false;
  std::set<std::string> experiment_names;
  for (int i = 0; i < study.experiment_size(); ++i) {
    if (study.experiment(i).name().empty()) {
      DVLOG(1) << study.name() << " is missing experiment " << i << " name";
      return false;
    }
    if (!experiment_names.insert(study.experiment(i).name()).second) {
      DVLOG(1) << study.name() << " has a repeated experiment name "
               << study.experiment(i).name();
      return false;
    }
    divisor += study.experiment(i).probability_weight();
    if (study.experiment(i).name() == default_group_name)
      found_default_group = true;
  }

  if (!found_default_group) {
    DVLOG(1) << study.name() << " is missing default experiment in its "
             << "experiment list";
    // The default group was not found in the list of groups. This study is not
    // valid.
    return false;
  }

  *total_probability = divisor;
  return true;
}

bool VariationsService::LoadTrialsSeedFromPref(
    PrefService* local_prefs,
    chrome_variations::TrialsSeed* seed) {
  std::string base64_seed_data = local_prefs->GetString(prefs::kVariationsSeed);
  std::string seed_data;

  // If the decode process fails, assume the pref value is corrupt, and clear
  // it.
  if (!base::Base64Decode(base64_seed_data, &seed_data) ||
      !seed->ParseFromString(seed_data)) {
    VLOG(1) << "Variations Seed data in local pref is corrupt, clearing the "
            << "pref.";
    local_prefs->ClearPref(prefs::kVariationsSeed);
    return false;
  }
  return true;
}

void VariationsService::CreateTrialFromStudy(
    const chrome_variations::Study& study,
    const base::Time& reference_date) {
  base::FieldTrial::Probability total_probability = 0;
  if (!ValidateStudyAndComputeTotalProbability(study, &total_probability))
    return;

  // The trial is created without specifying an expiration date because the
  // expiration check in field_trial.cc is based on the build date. Instead,
  // the expiration check using |reference_date| is done explicitly below.
  scoped_refptr<base::FieldTrial> trial(
      base::FieldTrialList::FactoryGetFieldTrial(
          study.name(), total_probability, study.default_experiment_name(),
          base::FieldTrialList::kExpirationYearInFuture, 1, 1, NULL));

  if (study.has_consistency() &&
      study.consistency() == chrome_variations::Study_Consistency_PERMANENT) {
    trial->UseOneTimeRandomization();
  }

  for (int i = 0; i < study.experiment_size(); ++i) {
    if (study.experiment(i).name() != study.default_experiment_name()) {
      trial->AppendGroup(study.experiment(i).name(),
                         study.experiment(i).probability_weight());
    }
  }

  // TODO(jwd): Add experiment_id association code.
  trial->SetForced();
  if (IsStudyExpired(study, reference_date))
    trial->Disable();
}
