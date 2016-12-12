// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".features.agefeature");

#include <vespa/searchlib/fef/featurenamebuilder.h>
#include <vespa/searchlib/fef/matchdata.h>
#include "agefeature.h"
#include "valuefeature.h"

using search::attribute::IAttributeVector;

namespace search {

typedef fef::FeatureNameBuilder FNB;

namespace features {

AgeExecutor::AgeExecutor(const IAttributeVector *attribute) :
    search::fef::FeatureExecutor(),
    _attribute(attribute),
    _buf()
{
    if (_attribute != NULL) {
        _buf.allocate(attribute->getMaxValueCount());
    }
}

void
AgeExecutor::execute(uint32_t docId)
{
    feature_t age = 10000000000.0;
    if (_attribute != NULL) {
        _buf.fill(*_attribute, docId);
        int64_t docTime = _buf[0];
        feature_t currTime = inputs().get_number(0);
        age = currTime - docTime;
        if (age < 0) {
            age = 0;
        }
    }
    outputs().set_number(0, age);
}

void
AgeBlueprint::visitDumpFeatures(const search::fef::IIndexEnvironment &,
                                search::fef::IDumpFeatureVisitor &) const
{
    // empty
}

bool
AgeBlueprint::setup(const search::fef::IIndexEnvironment &env,
                    const search::fef::ParameterList &params)
{
    _attribute = params[0].getValue();
    defineInput("now");

    describeOutput("out", "The age of the document, in seconds.");
    env.hintAttributeAccess(_attribute);
    return true;
}

search::fef::Blueprint::UP
AgeBlueprint::createInstance() const
{
    return search::fef::Blueprint::UP(new AgeBlueprint());
}

search::fef::FeatureExecutor &
AgeBlueprint::createExecutor(const search::fef::IQueryEnvironment &env, vespalib::Stash &stash) const
{
    // Get docdate attribute vector
    const IAttributeVector * attribute = env.getAttributeContext().getAttribute(_attribute);
    return stash.create<AgeExecutor>(attribute);
}

}
}
