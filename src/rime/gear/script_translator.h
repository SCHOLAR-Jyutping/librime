//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2011-08-07 GONG Chen <chen.sst@gmail.com>
//
#ifndef RIME_SCRIPT_TRANSLATOR_H_
#define RIME_SCRIPT_TRANSLATOR_H_

#include <rime/common.h>
#include <rime/translation.h>
#include <rime/translator.h>
#include <rime/algo/algebra.h>
#include <rime/gear/memory.h>
#include <rime/gear/translator_commons.h>

namespace rime {

class Code;
class Corrector;
struct DictEntry;
class Dictionary;
class Poet;
class UserDictionary;
struct SyllableGraph;

class ScriptTranslator : public Translator,
                         public Memory,
                         public TranslatorOptions {
 public:
  ScriptTranslator(const Ticket& ticket);

  virtual an<Translation> Query(const string& input, const Segment& segment);
  virtual bool Memorize(const CommitEntry& commit_entry);

  string FormatPreedit(const string& preedit);
  string Spell(const Code& code);
  string GetPrecedingText(size_t start) const;

  // options
  int max_homophones() const { return max_homophones_; }
  int spelling_hints() const { return spelling_hints_; }
  bool always_show_comments() const { return always_show_comments_; }
  bool enable_sentence() const { return enable_sentence_; }
  bool encode_commit_history() const { return encode_commit_history_; }

 protected:
  int max_homophones_ = 16;
  int spelling_hints_ = 4;
  bool always_show_comments_ = false;
  bool enable_correction_ = false;
  bool enable_sentence_ = true;
  bool encode_commit_history_ = true;
  bool combine_candidates_ = true;
  the<Corrector> corrector_;
  the<Poet> poet_;
};

class ScriptSyllabifier : public PhraseSyllabifier {
 public:
  ScriptSyllabifier(ScriptTranslator* translator,
                    Corrector* corrector,
                    const string& input,
                    size_t start)
      : translator_(translator),
        input_(input),
        start_(start),
        syllabifier_(translator->delimiters(),
                     translator->enable_completion(),
                     translator->strict_spelling()) {
    if (corrector) {
      syllabifier_.EnableCorrection(corrector);
    }
  }

  virtual Spans Syllabify(const Phrase* phrase);
  size_t BuildSyllableGraph(Prism& prism);
  string GetPreeditString(const Phrase& cand) const;
  string GetOriginalSpelling(const Phrase& cand) const;
  bool IsCandidateCorrection(const Phrase& cand) const;

  ScriptTranslator* translator() const { return translator_; }
  const SyllableGraph& syllable_graph() const { return syllable_graph_; }

 protected:
  ScriptTranslator* translator_;
  string input_;
  size_t start_;
  Syllabifier syllabifier_;
  SyllableGraph syllable_graph_;
};

}  // namespace rime

#endif  // RIME_SCRIPT_TRANSLATOR_H_
