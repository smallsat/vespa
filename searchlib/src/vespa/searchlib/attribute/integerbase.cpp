// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include "integerbase.h"
#include <vespa/searchlib/common/sort.h>
#include <vespa/searchlib/attribute/attributevector.hpp>
#include <vespa/document/fieldvalue/fieldvalue.h>

#include <vespa/log/log.h>
LOG_SETUP(".searchlib.attribute.integerbase");

namespace search {

IMPLEMENT_IDENTIFIABLE_ABSTRACT(IntegerAttribute, NumericAttribute);

IntegerAttribute::IntegerAttribute(const vespalib::string & name, const Config & c) :
    NumericAttribute(name, c),
    _changes()
{
}

uint32_t IntegerAttribute::clearDoc(DocId doc)
{
    uint32_t removed(0);
    if (hasMultiValue() && (doc < getNumDocs())) {
        removed = getValueCount(doc);
    }
    AttributeVector::clearDoc(_changes, doc);

    return removed;
}

uint32_t IntegerAttribute::get(DocId doc, WeightedString * s, uint32_t sz) const
{
    WeightedInt * v = new WeightedInt[sz];
    unsigned num(static_cast<const AttributeVector *>(this)->get(doc, v, sz));
    for(unsigned i(0); i < num; i++) {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "%" PRId64, v[i].getValue());
        s[i] = WeightedString(tmp, v[i].getWeight());
    }
    delete [] v;
    return num;
}

uint32_t IntegerAttribute::get(DocId doc, WeightedConstChar * v, uint32_t sz) const
{
    (void) doc;
    (void) v;
    (void) sz;
    return 0;
}

uint32_t IntegerAttribute::get(DocId doc, vespalib::string * s, uint32_t sz) const
{
    largeint_t * v = new largeint_t[sz];
    unsigned num(static_cast<const AttributeVector *>(this)->get(doc, v, sz));
    for(unsigned i(0); i < num; i++) {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "%" PRId64, v[i]);
        s[i] = tmp;
    }
    delete [] v;
    return num;
}

uint32_t IntegerAttribute::get(DocId doc, const char ** v, uint32_t sz) const
{
    (void) doc;
    (void) v;
    (void) sz;
    return 0;
}

bool IntegerAttribute::applyWeight(DocId doc, const FieldValue & fv, const ArithmeticValueUpdate & wAdjust)
{
    largeint_t v = fv.getAsLong();
    return AttributeVector::adjustWeight(_changes, doc, NumericChangeData<largeint_t>(v), wAdjust);
}

bool IntegerAttribute::apply(DocId doc, const ArithmeticValueUpdate & op)
{
    bool retval(doc < getNumDocs());
    if (retval) {
        retval = AttributeVector::applyArithmetic(_changes, doc, NumericChangeData<largeint_t>(0), op);
    }
    return retval;
}

}
