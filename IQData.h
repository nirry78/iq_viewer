#ifndef _IQ_DATA_H
#define _IQ_DATA_H

#include "Platform.h"
#include "IQDebug.h"

typedef struct
{
    double i, q;
    double angle;
    double unwrapped_angle;
    double demod;
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
        ComplexNumber m_DataMin;
        ComplexNumber m_DataMax;
        double m_AngleMin;
        double m_AngleMax;
        double m_UnwrappedAngleMin;
        double m_UnwrappedAngleMax;
        double m_DemodMin;
        double m_DemodMax;
        size_t m_DataCount;
        size_t m_DataSize;
        size_t m_DataIncrease;
    public:
        IQData(size_t expectedCount);
        virtual ~IQData();

        bool AddValue(double i, double q);
        void Dump(FILE *dst);
        size_t GetCount() { return m_DataCount; };
        bool GetMinValue(ValueType type, double *value);
        bool GetMaxValue(ValueType type, double *value);
        bool GetValue(size_t index, ValueType type, double *value);
        bool ReadFile(FILE *file);
        bool ProcessData(char *data, size_t dataLength);

};

#endif /* _IQ_DATA_H */