// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include "dense_tensor_view.h"

namespace vespalib {
namespace tensor {

/**
 * A mutable view to a dense tensor where all dimensions are indexed.
 */
class MutableDenseTensorView : public DenseTensorView
{
private:
    struct MutableValueType
    {
    private:
        eval::ValueType _type;
        std::vector<size_t *> _unboundDimSizes;

    public:
        MutableValueType(eval::ValueType type_in);
        const eval::ValueType &type() const { return _type; }
        void setUnboundDimensions(const uint32_t *unboundDimSizeBegin, const uint32_t *unboundDimSizeEnd) {
            const uint32_t *unboundDimSizePtr = unboundDimSizeBegin;
            for (auto unboundDimSize : _unboundDimSizes) {
                *unboundDimSize = *unboundDimSizePtr++;
            }
            assert(unboundDimSizePtr == unboundDimSizeEnd);
            (void) unboundDimSizeEnd;
        }
        void setUnboundDimensionsForEmptyTensor() {
            for (auto unboundDimSize : _unboundDimSizes) {
                *unboundDimSize = 1;
            }
        }
    };

    MutableValueType _concreteType;

public:
    MutableDenseTensorView(eval::ValueType type_in);
    MutableDenseTensorView(eval::ValueType type_in, CellsRef cells_in);
    void setCells(CellsRef cells_in) {
        _cellsRef = cells_in;
    }
    void setUnboundDimensions(const uint32_t *unboundDimSizeBegin, const uint32_t *unboundDimSizeEnd) {
        _concreteType.setUnboundDimensions(unboundDimSizeBegin, unboundDimSizeEnd);
    }
    void setUnboundDimensionsForEmptyTensor() {
        _concreteType.setUnboundDimensionsForEmptyTensor();
    }
};

} // namespace vespalib::tensor
} // namespace vespalib
