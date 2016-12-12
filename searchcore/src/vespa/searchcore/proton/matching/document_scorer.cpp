// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "document_scorer.h"

using search::feature_t;
using search::fef::FeatureResolver;
using search::fef::RankProgram;
using search::queryeval::SearchIterator;

namespace proton {
namespace matching {

namespace {

const feature_t *
extractScoreFeature(const RankProgram &rankProgram)
{
    FeatureResolver resolver(rankProgram.get_seeds());
    assert(resolver.num_features() == 1u);
    return resolver.resolve_number(0);
}

}

DocumentScorer::DocumentScorer(RankProgram &rankProgram,
                               SearchIterator &searchItr)
    : _rankProgram(rankProgram),
      _searchItr(searchItr),
      _scoreFeature(extractScoreFeature(rankProgram))
{
}

feature_t
DocumentScorer::score(uint32_t docId)
{
    return doScore(docId);
}

} // namespace proton::matching
} // namespace proton
