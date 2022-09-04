#include "IQData.h"

IQData::IQData(size_t expectedCount):
    m_DataSize(expectedCount),
    m_DataCount(0),
    m_Data(NULL),
    m_DataIncrease(128)
{
    if (expectedCount)
    {
        m_Data = new ComplexNumber[expectedCount];
    }
}

IQData::~IQData()
{

}

bool IQData::AddValue(double i, double q)
{
    bool result = false;

    if (m_DataCount >= m_DataSize)
    {
        ComplexNumber *data = new ComplexNumber[m_DataSize + m_DataIncrease];
        if (data)
        {
            memcpy(data, m_Data, sizeof(ComplexNumber) * m_DataCount);
            delete m_Data;
            m_Data = data;
            m_DataSize += m_DataIncrease;
        }
    }

    if (m_DataCount < m_DataSize)
    {
        m_Data[m_DataCount].i = i;
        m_Data[m_DataCount].q = q;
        m_DataCount++;
        result = true;
    }

    return result;
}

void IQData::Dump(FILE *dst)
{
    for (size_t index = 0; index < m_DataCount; index++)
    {
        double angle = atan2(m_Data[index].q, m_Data[index].i) * 360.0 / M_PI;
        fprintf(dst, "%d; %d; %.05g\n", (int32_t)m_Data[index].i, (int32_t)m_Data[index].q, angle);
    }
}

bool IQData::GetValue(size_t index, double *i, double *q)
{
    bool result = false;

    if (index < m_DataCount)
    {
        *i = m_Data[index].i;
        *q = m_Data[index].q;
        result = true;
    }

    return result;
}

bool IQData::ReadFile(FILE *file)
{
/*    char buffer[1024];

    while (fgets(buffer, sizeof(buffer) - 1, file) == EOF)
    {

    }*/

    return true;
}