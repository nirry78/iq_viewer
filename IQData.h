#ifndef _IQ_DATA_H
#define _IQ_DATA_H

#include "Platform.h"

typedef struct
{
    double i, q;
} ComplexNumber;

typedef enum {
    ValueTypeNone,
    ValueTypeI,
    ValueTypeQ,
    ValueTypePower,
    ValueTypeDemod,
    ValueTypePhase,
    ValueTypeUnwrappedPhase,
} ValueType;

class IQData
{
    private:
        ComplexNumber *m_Data;
        size_t m_DataCount;
        size_t m_DataSize;
        size_t m_DataIncrease;
    public:
        IQData(size_t expectedCount);
        virtual ~IQData();

        bool AddValue(double i, double q);
        void Dump(FILE *dst);
        size_t GetCount() { return m_DataCount; };
        bool GetValue(size_t index, ValueType type, double *value);
        bool ReadFile(FILE *file);

};

#endif /* _IQ_DATA_H */