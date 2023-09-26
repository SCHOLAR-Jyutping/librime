//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2011-05-21 GONG Chen <chen.sst@gmail.com>
//
#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/gear/translator_commons.h>

namespace rime {

int Translation::Compare(an<Translation> other,
                         const CandidateList& candidates) {
  if (!other || other->exhausted())
    return -1;
  if (exhausted())
    return 1;
  auto ours = Peek();
  auto theirs = other->Peek();
  if (!ours || !theirs)
    return 1;
  return ours->compare(*theirs);
}

bool UniqueTranslation::Next() {
  if (exhausted())
    return false;
  set_exhausted(true);
  return true;
}

an<Candidate> UniqueTranslation::Peek() {
  if (exhausted())
    return nullptr;
  return candidate_;
}

FifoTranslation::FifoTranslation() {
  set_exhausted(true);
}

bool FifoTranslation::Next() {
  if (exhausted())
    return false;
  if (++cursor_ >= candies_.size())
    set_exhausted(true);
  return true;
}

an<Candidate> FifoTranslation::Peek() {
  if (exhausted())
    return nullptr;
  return candies_[cursor_];
}

void FifoTranslation::Append(an<Candidate> candy) {
  candies_.push_back(candy);
  set_exhausted(false);
}

UnionTranslation::UnionTranslation() {
  set_exhausted(true);
}

bool UnionTranslation::Next() {
  if (exhausted())
    return false;
  translations_.front()->Next();
  if (translations_.front()->exhausted()) {
    translations_.pop_front();
    if (translations_.empty()) {
      set_exhausted(true);
    }
  }
  return true;
}

an<Candidate> UnionTranslation::Peek() {
  if (exhausted())
    return nullptr;
  return translations_.front()->Peek();
}

UnionTranslation& UnionTranslation::operator+=(an<Translation> t) {
  if (t && !t->exhausted()) {
    translations_.push_back(t);
    set_exhausted(false);
  }
  return *this;
}

an<UnionTranslation> operator+(an<Translation> x, an<Translation> y) {
  auto z = New<UnionTranslation>();
  *z += x;
  *z += y;
  return z->exhausted() ? nullptr : z;
}

// MergedTranslation

MergedTranslation::MergedTranslation(const CandidateList& candidates)
    : previous_candidates_(candidates) {
  set_exhausted(true);
}

bool MergedTranslation::Next() {
  if (exhausted()) {
    return false;
  }
  translations_[elected_]->Next();
  if (translations_[elected_]->exhausted()) {
    DLOG(INFO) << "translation #" << elected_ << " has been exhausted.";
    translations_.erase(translations_.begin() + elected_);
  }
  Elect();
  return !exhausted();
}

an<Candidate> MergedTranslation::Peek() {
  if (exhausted()) {
    return nullptr;
  }
  return translations_[elected_]->Peek();
}

void MergedTranslation::Elect() {
  if (translations_.empty()) {
    set_exhausted(true);
    return;
  }
  size_t k = 0;
  for (; k < translations_.size(); ++k) {
    const auto& current = translations_[k];
    const auto& next =
        k + 1 < translations_.size() ? translations_[k + 1] : nullptr;
    if (current->Compare(next, previous_candidates_) <= 0) {
      if (current->exhausted()) {
        translations_.erase(translations_.begin() + k);
        k = 0;
        continue;
      }
      break;
    }
  }
  elected_ = k;
  if (k >= translations_.size()) {
    DLOG(WARNING) << "failed to elect a winner translation.";
    set_exhausted(true);
  } else {
    set_exhausted(false);
  }
}

MergedTranslation& MergedTranslation::operator+=(an<Translation> t) {
  if (t && !t->exhausted()) {
    translations_.push_back(t);
    Elect();
  }
  return *this;
}

// CacheTranslation

CacheTranslation::CacheTranslation(an<Translation> translation)
    : translation_(translation) {
  set_exhausted(!translation_ || translation_->exhausted());
}

bool CacheTranslation::Next() {
  if (exhausted())
    return false;
  cache_.reset();
  translation_->Next();
  if (translation_->exhausted()) {
    set_exhausted(true);
  }
  return true;
}

an<Candidate> CacheTranslation::Peek() {
  if (exhausted())
    return nullptr;
  if (!cache_) {
    cache_ = translation_->Peek();
  }
  return cache_;
}

// DistinctTranslation

DistinctTranslation::DistinctTranslation(an<Translation> translation,
                                         const bool combine_candidates)
    : translation_(New<CacheTranslation>(translation)),
      combine_candidates_(combine_candidates) {
  set_exhausted(!translation_ || translation_->exhausted());
}

an<Candidate> DistinctTranslation::Peek() {
  if (exhausted())
    return nullptr;
  if (cache_ || Next())
    return cache_;
  return nullptr;
}

bool DistinctTranslation::Next() {
  if (exhausted())
    return false;
  while (true) {
    cache_ = translation_->Peek();
    translation_->Next();
    if (!cache_) {
      set_exhausted(true);
      return false;
    }
    pair<string, string> pair = {cache_->text(), cache_->comment()};
    if (!AlreadyHas(pair)) {
      candidate_set_.insert(pair);
      break;
    }
  }
  an<Phrase> phrase = As<Phrase>(Candidate::GetGenuineCandidate(cache_));
  if (phrase && combine_candidates_) {
    string comment = phrase->comment();
    while (!translation_->exhausted()) {
      an<Candidate> cand = translation_->Peek();
      pair<string, string> pair = {cand->text(), cand->comment()};
      if (AlreadyHas(pair)) {
        translation_->Next();
        continue;
      }
      if (cand->text() == phrase->text()) {
        comment += (comment.empty() || cand->comment().empty() ? "" : "; ") + cand->comment();
        translation_->Next();
        candidate_set_.insert(pair);
      } else {
        break;
      }
    }
    phrase->set_comment(comment);
  }
  if (translation_->exhausted()) {
    set_exhausted(true);
  }
  return true;
}

bool DistinctTranslation::AlreadyHas(const pair<string, string>& candidate) const {
  return candidate_set_.find(candidate) != candidate_set_.end();
}

// PrefetchTranslation

PrefetchTranslation::PrefetchTranslation(an<Translation> translation)
    : translation_(translation) {
  set_exhausted(!translation_ || translation_->exhausted());
}

bool PrefetchTranslation::Next() {
  if (exhausted()) {
    return false;
  }
  if (!cache_.empty()) {
    cache_.pop_front();
  } else {
    translation_->Next();
  }
  if (cache_.empty() && translation_->exhausted()) {
    set_exhausted(true);
  }
  return true;
}

an<Candidate> PrefetchTranslation::Peek() {
  if (exhausted()) {
    return nullptr;
  }
  if (!cache_.empty() || Replenish()) {
    return cache_.front();
  } else {
    return translation_->Peek();
  }
}

}  // namespace rime
