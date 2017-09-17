// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "zcpostingiterators.h"
#include <vespa/searchlib/fef/termfieldmatchdataarray.h>
#include <vespa/searchlib/bitcompression/posocccompression.h>

namespace search {

namespace diskindex {

using search::fef::TermFieldMatchDataArray;
using search::bitcompression::FeatureDecodeContext;
using search::bitcompression::FeatureEncodeContext;
using queryeval::RankedSearchIteratorBase;

#define DEBUG_ZCPOSTING_PRINTF 0
#define DEBUG_ZCPOSTING_ASSERT 0

ZcIteratorBase::ZcIteratorBase(const TermFieldMatchDataArray &matchData, Position start, uint32_t docIdLimit) :
    RankedSearchIteratorBase(matchData),
    _docIdLimit(docIdLimit),
    _start(start)
{ }

void
ZcIteratorBase::initRange(uint32_t beginid, uint32_t endid)
{
    uint32_t prev = getDocId();
    setEndId(endid);
    if ((beginid <= prev) || (prev == 0)) {
        rewind(_start);
        readWordStart(getDocIdLimit());
    }
    seek(beginid);
}


template <bool bigEndian>
Zc4RareWordPostingIterator<bigEndian>::
Zc4RareWordPostingIterator(const TermFieldMatchDataArray &matchData, Position start, uint32_t docIdLimit)
    : ZcIteratorBase(matchData, start, docIdLimit),
      _decodeContext(NULL),
      _residue(0),
      _prevDocId(0),
      _numDocs(0)
{ }


template <bool bigEndian>
void
Zc4RareWordPostingIterator<bigEndian>::doSeek(uint32_t docId)
{
    typedef FeatureEncodeContext<bigEndian> EC;
    uint32_t length;
    uint64_t val64;

    uint32_t oDocId = getDocId();

    UC64_DECODECONTEXT_CONSTRUCTOR(o, _decodeContext->_);
    if (getUnpacked()) {
        clearUnpacked();
        if (__builtin_expect(--_residue == 0, false))
            goto atbreak;
        UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_DELTA_DOCID, EC);
        oDocId += 1 + static_cast<uint32_t>(val64);
#if DEBUG_ZCPOSTING_PRINTF
        printf("Decode docId=%d\n",
               oDocId);
#endif
    }
    while (__builtin_expect(oDocId < docId, true)) {
        UC64_DECODECONTEXT_STORE(o, _decodeContext->_);
        _decodeContext->skipFeatures(1);
        UC64_DECODECONTEXT_LOAD(o, _decodeContext->_);
        if (__builtin_expect(--_residue == 0, false))
            goto atbreak;
        UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_DELTA_DOCID, EC);
        oDocId += 1 + static_cast<uint32_t>(val64);
#if DEBUG_ZCPOSTING_PRINTF
        printf("Decode docId=%d\n",
               oDocId);
#endif
    }
    UC64_DECODECONTEXT_STORE(o, _decodeContext->_);
    setDocId(oDocId);
    return;
 atbreak:
    setAtEnd(); // Mark end of data
    return;
}


template <bool bigEndian>
void
Zc4RareWordPostingIterator<bigEndian>::doUnpack(uint32_t docId)
{
    if (!_matchData.valid() || getUnpacked())
        return;
    assert(docId == getDocId());
    _decodeContext->unpackFeatures(_matchData, docId);
    setUnpacked();
}

template <bool bigEndian>
void Zc4RareWordPostingIterator<bigEndian>::rewind(Position start)
{
    _decodeContext->setPosition(start);
}

template <bool bigEndian>
void
Zc4RareWordPostingIterator<bigEndian>::readWordStart(uint32_t docIdLimit)
{
    (void) docIdLimit;
    typedef FeatureEncodeContext<bigEndian> EC;
    UC64_DECODECONTEXT_CONSTRUCTOR(o, _decodeContext->_);
    uint32_t length;
    uint64_t val64;

    UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_NUMDOCS, EC);

    _numDocs = static_cast<uint32_t>(val64) + 1;
    UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_DELTA_DOCID, EC);
    uint32_t docId = static_cast<uint32_t>(val64) + 1;
    UC64_DECODECONTEXT_STORE(o, _decodeContext->_);

    setDocId(docId);
    _residue = _numDocs;
    clearUnpacked();
}


template <bool bigEndian>
ZcRareWordPostingIterator<bigEndian>::
ZcRareWordPostingIterator(const TermFieldMatchDataArray &matchData, Position start, uint32_t docIdLimit)
    : Zc4RareWordPostingIterator<bigEndian>(matchData, start, docIdLimit),
      _docIdK(0)
{
}


template <bool bigEndian>
void
ZcRareWordPostingIterator<bigEndian>::doSeek(uint32_t docId)
{
    typedef FeatureEncodeContext<bigEndian> EC;
    uint32_t length;
    uint64_t val64;

    uint32_t oDocId = getDocId();

    UC64_DECODECONTEXT_CONSTRUCTOR(o, _decodeContext->_);
    if (getUnpacked()) {
        clearUnpacked();
        if (__builtin_expect(--_residue == 0, false))
            goto atbreak;
        UC64_DECODEEXPGOLOMB_NS(o, _docIdK, EC);
        oDocId += 1 + static_cast<uint32_t>(val64);
#if DEBUG_ZCPOSTING_PRINTF
        printf("Decode docId=%d\n",
               oDocId);
#endif
    }
    while (__builtin_expect(oDocId < docId, true)) {
        UC64_DECODECONTEXT_STORE(o, _decodeContext->_);
        _decodeContext->skipFeatures(1);
        UC64_DECODECONTEXT_LOAD(o, _decodeContext->_);
        if (__builtin_expect(--_residue == 0, false))
            goto atbreak;
        UC64_DECODEEXPGOLOMB_NS(o, _docIdK, EC);
        oDocId += 1 + static_cast<uint32_t>(val64);
#if DEBUG_ZCPOSTING_PRINTF
        printf("Decode docId=%d\n",
               oDocId);
#endif
    }
    UC64_DECODECONTEXT_STORE(o, _decodeContext->_);
    setDocId(oDocId);
    return;
 atbreak:
    setAtEnd();   // Mark end of data
    return;
}


template <bool bigEndian>
void
ZcRareWordPostingIterator<bigEndian>::readWordStart(uint32_t docIdLimit)
{
    typedef FeatureEncodeContext<bigEndian> EC;
    UC64_DECODECONTEXT_CONSTRUCTOR(o, _decodeContext->_);
    uint32_t length;
    uint64_t val64;

    UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_NUMDOCS, EC);
    _numDocs = static_cast<uint32_t>(val64) + 1;
    _docIdK = EC::calcDocIdK(_numDocs, docIdLimit);
    UC64_DECODEEXPGOLOMB_NS(o, _docIdK, EC);
    uint32_t docId = static_cast<uint32_t>(val64) + 1;
    UC64_DECODECONTEXT_STORE(o, _decodeContext->_);

    setDocId(docId);
    _residue = _numDocs;
    clearUnpacked();
}

ZcPostingIteratorBase::ZcPostingIteratorBase(const TermFieldMatchDataArray &matchData, Position start, uint32_t docIdLimit)
    : ZcIteratorBase(matchData, start, docIdLimit),
      _valI(NULL),
      _valIBase(NULL),
      _featureSeekPos(0),
      _l1(),
      _l2(),
      _l3(),
      _l4(),
      _chunk(),
      _featuresSize(0),
      _hasMore(false),
      _chunkNo(0)
{
}

template <bool bigEndian>
ZcPostingIterator<bigEndian>::
ZcPostingIterator(uint32_t minChunkDocs,
                  bool dynamicK,
                  const PostingListCounts &counts,
                  const search::fef::TermFieldMatchDataArray &matchData,
                  Position start, uint32_t docIdLimit)
    : ZcPostingIteratorBase(matchData, start, docIdLimit),
      _decodeContext(NULL),
      _minChunkDocs(minChunkDocs),
      _docIdK(0),
      _dynamicK(dynamicK),
      _numDocs(0),
      _featuresValI(NULL),
      _featuresBitOffset(0),
      _counts(counts)
{ }


template <bool bigEndian>
void
ZcPostingIterator<bigEndian>::readWordStart(uint32_t docIdLimit)
{
    typedef FeatureEncodeContext<bigEndian> EC;
    DecodeContextBase &d = *_decodeContext;
    UC64_DECODECONTEXT_CONSTRUCTOR(o, d._);
    uint32_t length;
    uint64_t val64;

    uint32_t prevDocId = _hasMore ? _chunk._lastDocId : 0u;
    UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_NUMDOCS, EC);

    _numDocs = static_cast<uint32_t>(val64) + 1;
    bool hasMore = false;
    if (__builtin_expect(_numDocs >= _minChunkDocs, false)) {
        if (bigEndian) {
            hasMore = static_cast<int64_t>(oVal) < 0;
            oVal <<= 1;
            length = 1;
        } else {
            hasMore = (oVal & 1) != 0;
            oVal >>= 1;
            length = 1;
        }
        UC64_READBITS_NS(o, EC);
    }
    if (_dynamicK)
        _docIdK = EC::calcDocIdK((_hasMore || hasMore) ? 1 : _numDocs, docIdLimit);
    UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_DOCIDSSIZE, EC);
    uint32_t docIdsSize = val64 + 1;
    UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_L1SKIPSIZE, EC);
    uint32_t l1SkipSize = val64;
    uint32_t l2SkipSize = 0;
    if (l1SkipSize != 0) {
        UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_L2SKIPSIZE, EC);
        l2SkipSize = val64;
    }
    uint32_t l3SkipSize = 0;
    if (l2SkipSize != 0) {
        UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_L3SKIPSIZE, EC);
        l3SkipSize = val64;
    }
    uint32_t l4SkipSize = 0;
    if (l3SkipSize != 0) {
        UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_L4SKIPSIZE, EC);
        l4SkipSize = val64;
    }
    UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_FEATURESSIZE, EC);
    _featuresSize = val64;
    if (_dynamicK) {
        UC64_DECODEEXPGOLOMB_NS(o, _docIdK, EC);
    } else {
        UC64_DECODEEXPGOLOMB_NS(o, K_VALUE_ZCPOSTING_LASTDOCID, EC);
    }
    _chunk._lastDocId = docIdLimit - 1 - val64;
    if (_hasMore || hasMore) {
        if (!_counts._segments.empty()) {
            assert(_chunk._lastDocId == _counts._segments[_chunkNo]._lastDoc);
        }
    }

    uint64_t bytePad = oPreRead & 7;
    if (bytePad > 0) {
        length = bytePad;
        UC64_READBITS_NS(o, EC);
    }

    UC64_DECODECONTEXT_STORE(o, d._);
    assert((d.getBitOffset() & 7) == 0);
    const uint8_t *bcompr = d.getByteCompr();
    _valIBase = _valI = bcompr;
    bcompr += docIdsSize;
    _l1.setup(prevDocId, _chunk._lastDocId, bcompr, l1SkipSize);
    _l2.setup(prevDocId, _chunk._lastDocId, bcompr, l2SkipSize);
    _l3.setup(prevDocId, _chunk._lastDocId, bcompr, l3SkipSize);
    _l4.setup(prevDocId, _chunk._lastDocId, bcompr, l4SkipSize);
    _l1.postSetup(*this);
    _l2.postSetup(_l1);
    _l3.postSetup(_l2);
    _l4.postSetup(_l3);
    d.setByteCompr(bcompr);
    _hasMore = hasMore;
    // Save information about start of next chunk
    _featuresValI = d.getCompr();
    _featuresBitOffset = d.getBitOffset();
    _featureSeekPos = 0;
    clearUnpacked();
    // Unpack first docid delta in chunk
    nextDocId(prevDocId);
#if DEBUG_ZCPOSTING_PRINTF
    printf("Decode docId=%d\n", getDocId());
#endif
}


void
ZcPostingIteratorBase::doChunkSkipSeek(uint32_t docId)
{
    while (docId > _chunk._lastDocId && _hasMore) {
        // Skip to start of next chunk
        _featureSeekPos = 0;
        featureSeek(_featuresSize);
        _chunkNo++;
        readWordStart(getDocIdLimit()); // Read word start for next chunk
    }
    if (docId > _chunk._lastDocId) {
        _l4._skipDocId = _l3._skipDocId = _l2._skipDocId = _l1._skipDocId = search::endDocId;
        setAtEnd();
    }
}


void
ZcPostingIteratorBase::doL4SkipSeek(uint32_t docId)
{
    uint32_t lastL4SkipDocId;

    if (__builtin_expect(docId > _chunk._lastDocId, false)) {
        doChunkSkipSeek(docId);
        if (docId <= _l4._skipDocId)
            return;
    }
    do {
        lastL4SkipDocId = _l4._skipDocId;
        _l4.decodeSkipEntry();
        _l4.nextDocId();
#if DEBUG_ZCPOSTING_PRINTF
        printf("L4Decode docId %d, docIdPos %d,"
               "l1SkipPos %d, l2SkipPos %d, l3SkipPos %d, nextDocId %d\n",
               lastL4SkipDocId,
               (int) (_l4._docIdPos - _valIBase),
               (int) (_l4._l1Pos - _l1._valIBase),
               (int) (_l4._l2Pos - _l2._valIBase),
               (int) (_l4._l3Pos - _l3._valIBase),
               _l4._skipDocId);
#endif
    } while (docId > _l4._skipDocId);
    _valI = _l1._docIdPos = _l2._docIdPos = _l3._docIdPos =
            _l4._docIdPos;
    _l1._skipFeaturePos = _l2._skipFeaturePos = _l3._skipFeaturePos =
                        _l4._skipFeaturePos;
    _l1._skipDocId = _l2._skipDocId = _l3._skipDocId = lastL4SkipDocId;
    _l1._valI = _l2._l1Pos = _l3._l1Pos = _l4._l1Pos;
    _l2._valI = _l3._l2Pos = _l4._l2Pos;
    _l3._valI = _l4._l3Pos;
    nextDocId(lastL4SkipDocId);
    _l1.nextDocId();
    _l2.nextDocId();
    _l3.nextDocId();
#if DEBUG_ZCPOSTING_PRINTF
    printf("L4Seek, docId %d docIdPos %d"
           " L1SkipPos %d L2SkipPos %d L3SkipPos %d, nextDocId %d\n",
           lastL4SkipDocId,
           (int) (_l4._docIdPos - _valIBase),
           (int) (_l4._l1Pos - _l1._valIBase),
           (int) (_l4._l2Pos - _l2._valIBase),
           (int) (_l4._l3Pos - _l3._valIBase),
           _l4._skipDocId);
#endif
    _featureSeekPos = _l4._skipFeaturePos;
    clearUnpacked();
}


void
ZcPostingIteratorBase::doL3SkipSeek(uint32_t docId)
{
    uint32_t lastL3SkipDocId;

    if (__builtin_expect(docId > _l4._skipDocId, false)) {
        doL4SkipSeek(docId);
        if (docId <= _l3._skipDocId)
            return;
    }
    do {
        lastL3SkipDocId = _l3._skipDocId;
        _l3.decodeSkipEntry();
        _l3.nextDocId();
#if DEBUG_ZCPOSTING_PRINTF
        printf("L3Decode docId %d, docIdPos %d,"
               "l1SkipPos %d, l2SkipPos %d, nextDocId %d\n",
               lastL3SkipDocId,
               (int) (_l3._docIdPos - _valIBase),
               (int) (_l3._l1Pos - _l1._valIBase),
               (int) (_l3._l2Pos - _l2._valIBase),
               _l3._skipDocId);
#endif
    } while (docId > _l3._skipDocId);
    _valI = _l1._docIdPos = _l2._docIdPos = _l3._docIdPos;
    _l1._skipFeaturePos = _l2._skipFeaturePos = _l3._skipFeaturePos;
    _l1._skipDocId = _l2._skipDocId = lastL3SkipDocId;
    _l1._valI = _l2._l1Pos = _l3._l1Pos;
    _l2._valI = _l3._l2Pos;
    nextDocId(lastL3SkipDocId);
    _l1.nextDocId();
    _l2.nextDocId();
#if DEBUG_ZCPOSTING_PRINTF
    printf("L3Seek, docId %d docIdPos %d"
           " L1SkipPos %d L2SkipPos %d, nextDocId %d\n",
           lastL3SkipDocId,
           (int) (_l3._docIdPos - _valIBase),
           (int) (_l3._l1Pos - _l1._valIBase),
           (int) (_l3._l2Pos - _l2._valIBase),
           _l3._skipDocId);
#endif
    _featureSeekPos = _l3._skipFeaturePos;
    clearUnpacked();
}


void
ZcPostingIteratorBase::doL2SkipSeek(uint32_t docId)
{
    uint32_t lastL2SkipDocId;

    if (__builtin_expect(docId > _l3._skipDocId, false)) {
        doL3SkipSeek(docId);
        if (docId <= _l2._skipDocId)
            return;
    }
    do {
        lastL2SkipDocId = _l2._skipDocId;
        _l2.decodeSkipEntry();
        _l2.nextDocId();
#if DEBUG_ZCPOSTING_PRINTF
        printf("L2Decode docId %d, docIdPos %d, l1SkipPos %d, nextDocId %d\n",
               lastL2SkipDocId,
               (int) (_l2._docIdPos - _valIBase),
               (int) (_l2._l1Pos - _l1._valIBase),
               _l2._skipDocId);
#endif
    } while (docId > _l2._skipDocId);
    _valI = _l1._docIdPos = _l2._docIdPos;
    _l1._skipFeaturePos = _l2._skipFeaturePos;
    _l1._skipDocId = lastL2SkipDocId;
    _l1._valI = _l2._l1Pos;
    nextDocId(lastL2SkipDocId);
    _l1.nextDocId();
#if DEBUG_ZCPOSTING_PRINTF
    printf("L2Seek, docId %d docIdPos %d L1SkipPos %d, nextDocId %d\n",
           lastL2SkipDocId,
           (int) (_l2._docIdPos - _valIBase),
           (int) (_l2._l1Pos - _l1._valIBase),
           _l2._skipDocId);
#endif
    _featureSeekPos = _l2._skipFeaturePos;
    clearUnpacked();
}


void
ZcPostingIteratorBase::doL1SkipSeek(uint32_t docId)
{
    uint32_t lastL1SkipDocId;
    if (__builtin_expect(docId > _l2._skipDocId, false)) {
        doL2SkipSeek(docId);
        if (docId <= _l1._skipDocId)
            return;
    }
    do {
        lastL1SkipDocId = _l1._skipDocId;
        _l1.decodeSkipEntry();
        _l1.nextDocId();
#if DEBUG_ZCPOSTING_PRINTF
        printf("L1Decode docId %d, docIdPos %d, L1SkipPos %d, nextDocId %d\n",
               lastL1SkipDocId,
               (int) (_l1._docIdPos - _valIBase),
               (int) (_l1._valI - _l1._valIBase),
                _l1._skipDocId);
#endif
    } while (docId > _l1._skipDocId);
    _valI = _l1._docIdPos;
    nextDocId(lastL1SkipDocId);
#if DEBUG_ZCPOSTING_PRINTF
    printf("L1SkipSeek, docId %d docIdPos %d, nextDocId %d\n",
           lastL1SkipDocId,
           (int) (_l1._docIdPos - _valIBase),
           _l1._skipDocId);
#endif
    _featureSeekPos = _l1._skipFeaturePos;
    clearUnpacked();
}


void
ZcPostingIteratorBase::doSeek(uint32_t docId)
{
    if (docId > _l1._skipDocId) {
        doL1SkipSeek(docId);
    }
    uint32_t oDocId = getDocId();
#if DEBUG_ZCPOSTING_ASSERT
    assert(oDocId <= _l1._skipDocId);
    assert(docId <= _l1._skipDocId);
    assert(oDocId <= _l2._skipDocId);
    assert(docId <= _l2._skipDocId);
    assert(oDocId <= _l3._skipDocId);
    assert(docId <= _l3._skipDocId);
    assert(oDocId <= _l4._skipDocId);
    assert(docId <= _l4._skipDocId);
#endif
    const uint8_t *oCompr = _valI;
    while (__builtin_expect(oDocId < docId, true)) {
#if DEBUG_ZCPOSTING_ASSERT
        assert(oDocId <= _l1._skipDocId);
        assert(oDocId <= _l2._skipDocId);
        assert(oDocId <= _l3._skipDocId);
        assert(oDocId <= _l4._skipDocId);
#endif
        ZCDECODE(oCompr, oDocId += 1 +);
#if DEBUG_ZCPOSTING_PRINTF
        printf("Decode docId=%d\n",
               oDocId);
#endif
        incNeedUnpack();
    }
    _valI = oCompr;
    setDocId(oDocId);
    return;
}


template <bool bigEndian>
void
ZcPostingIterator<bigEndian>::doUnpack(uint32_t docId)
{
    if (!_matchData.valid() || getUnpacked())
        return;
    if (_featureSeekPos != 0) {
        // Handle deferred feature position seek now.
        featureSeek(_featureSeekPos);
        _featureSeekPos = 0;
    }
    assert(docId == getDocId());
    uint32_t needUnpack = getNeedUnpack();
    if (needUnpack > 1)
        _decodeContext->skipFeatures(needUnpack - 1);
    _decodeContext->unpackFeatures(_matchData, docId);
    setUnpacked();
}

template <bool bigEndian>
void ZcPostingIterator<bigEndian>::rewind(Position start)
{
    _decodeContext->setPosition(start);
    _hasMore = false;
    _chunk._lastDocId = 0;
    _chunkNo = 0;
}


template class Zc4RareWordPostingIterator<true>;
template class Zc4RareWordPostingIterator<false>;

template class ZcPostingIterator<true>;
template class ZcPostingIterator<false>;

template class ZcRareWordPostingIterator<true>;
template class ZcRareWordPostingIterator<false>;

} // namespace diskindex

} // namespace search
