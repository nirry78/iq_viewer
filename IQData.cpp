#include "IQData.h"

typedef enum
{
    TOKEN_NEWLINE,
    TOKEN_VALUE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_WHITESPACE,
    TOKEN_DASH,
    TOKEN_OTHER,
} Token;

const char *tokenToText[] =
{
    "NEWLINE",
    "VALUE",
    "COMMA",
    "DOT",
    "WHITESPACE",
    "OTHER",
};

typedef enum
{
    DECODE_STATE_IDLE,
    DECODE_STATE_I,
    DECODE_STATE_I_WHITESPACE,
    DECODE_STATE_Q_PENDING,
    DECODE_STATE_Q,
    DECODE_STATE_IGNORE,
    DECODE_STATE_ERROR
} DecodeState;

const char *stateToText[] =
{
    "IDLE",
    "I",
    "I_WHITESPACE",
    "Q_PENDING",
    "Q",
    "IGNORE",
    "ERROR"
};

IQData::IQData(size_t expectedCount):
    m_DataSize(expectedCount),
    m_DataMin({ 0.0,  0.0 }),
    m_DataMax({ 0.0,  0.0 }),
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

    LOGD("AddValue (I: %g, Q: %g)", i, q);

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
        m_DataMin.i = i < m_DataMin.i ? i : m_DataMin.i;
        m_DataMax.i = i > m_DataMax.i ? i : m_DataMax.i;
        m_DataMin.q = q < m_DataMin.q ? q : m_DataMin.q;
        m_DataMax.q = q > m_DataMax.q ? q : m_DataMax.q;

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

bool IQData::GetMinValue(ValueType type, double *value)
{
    bool result = true;

    switch (type)
    {
        case ValueTypeI:
        {
            *value = m_DataMin.i;
            break;
        }
        case ValueTypeQ:
        {
            *value = m_DataMin.q;
            break;
        }
        default:
        {
            *value = 0.0;
            result = false;
            break;
        }
    }

    return result;
}
bool IQData::GetMaxValue(ValueType type, double *value)
{
    bool result = true;

    switch (type)
    {
        case ValueTypeI:
        {
            *value = m_DataMax.i;
            break;
        }
        case ValueTypeQ:
        {
            *value = m_DataMax.q;
            break;
        }
        default:
        {
            *value = 0.0;
            result = false;
            break;
        }
    }

    return result;
}

bool IQData::GetValue(size_t index, ValueType type, double *value)
{
    bool result = index < m_DataCount;

    if (result)
    {
        switch (type)
        {
            case ValueTypeI:
            {
                *value = m_Data[index].i;
                break;
            }
            case ValueTypeQ:
            {
                *value = m_Data[index].q;
                break;
            }
            case ValueTypePower:
            {
                *value = sqrt((m_Data[index].i * m_Data[index].i) + (m_Data[index].q * m_Data[index].q));
                break;
            }
            case ValueTypePhase:
            {
                *value = atan2(m_Data[index].q, m_Data[index].i) * 360.0 / M_PI;
                LOGV("ValueTypePhase: %g", *value);
                break;
            }
            default:
            {
                *value = 0.0;
                break;
            }
        }
    }

    return result;
}

bool IQData::ProcessData(char *data, size_t dataLength)
{
    DecodeState state = DECODE_STATE_IDLE;
    int64_t value_i64 = 0, value_i = 0, value_q = 0;
    bool negative = false;
    double value_double = 0.0;

    for (size_t index = 0; index < dataLength && state != DECODE_STATE_ERROR; index++)
    {
        const char c = data[index];
        size_t value;
        Token token;

        switch (c)
        {
            case '\n':
            case '\r':
            {
                token = TOKEN_NEWLINE;
                break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                value = (size_t)(c - '0');
                token = TOKEN_VALUE;
                break;
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            {
                value = (size_t)(c - ('a' + 10));
                token = TOKEN_VALUE;
                break;
            }
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            {
                value = (size_t)(c - ('A' + 10));
                token = TOKEN_VALUE;
                break;
            }
            case ',':
            {
                token = TOKEN_COMMA;
                break;
            }
            case '.':
            {
                token = TOKEN_DOT;
                break;
            }
            case '\t':
            case ' ':
            {
                token = TOKEN_WHITESPACE;
                break;
            }
            case '-':
            {
                token = TOKEN_DASH;
                break;
            }
            default:
            {
                token = TOKEN_OTHER;
                break;
            }
        }

        LOGD("Decode (state: %s, token: %s, input: %u)", stateToText[state], tokenToText[token], c);

        switch (state)
        {
            case DECODE_STATE_IDLE:
            {
                if (token == TOKEN_VALUE)
                {
                    value_i64 = value;
                    negative = false;
                    state = DECODE_STATE_I;
                }
                else if (token == TOKEN_DASH)
                {
                    value_i64 = 0;
                    negative = true;
                    state = DECODE_STATE_I;
                }
                break;
            }
            case DECODE_STATE_I:
            {
                if (token == TOKEN_VALUE)
                {
                    value_i64 = value + (value_i64 * 10);
                }
                else
                {
                    value_i = negative ? -value_i64 : value_i64;
                    negative = false;
                    value_i64 = 0;
                    negative = false;

                    if (token == TOKEN_WHITESPACE)
                    {
                        state = DECODE_STATE_I_WHITESPACE;
                    }
                    else if (token == TOKEN_NEWLINE)
                    {
                        state = DECODE_STATE_IDLE;
                    }
                    else if (token == TOKEN_COMMA)
                    {
                        state = DECODE_STATE_Q_PENDING;
                    }
                    else
                    {
                        state = DECODE_STATE_ERROR;
                    }
                }
                break;
            }
            case DECODE_STATE_I_WHITESPACE:
            {
                if (token == TOKEN_WHITESPACE)
                {
                    state = DECODE_STATE_I_WHITESPACE;
                }
                else if (token == TOKEN_NEWLINE)
                {
                    state = DECODE_STATE_IDLE;
                }
                else if (token == TOKEN_COMMA)
                {
                    state = DECODE_STATE_Q_PENDING;
                }
                else
                {
                    state = DECODE_STATE_ERROR;
                }
                break;
            }
            case DECODE_STATE_Q_PENDING:
            {
                if (token == TOKEN_VALUE)
                {
                    value_i64 = value;
                    state = DECODE_STATE_Q;
                }
                else if (token == TOKEN_DASH)
                {
                    value_i64 = 0;
                    negative = true;
                    state = DECODE_STATE_Q;
                }
                else if (token == TOKEN_WHITESPACE)
                {
                    state = DECODE_STATE_Q_PENDING;
                }
                else if (token == TOKEN_NEWLINE)
                {
                    state = DECODE_STATE_IDLE;
                }
                else
                {
                    state = DECODE_STATE_ERROR;
                }
                break;
            }
            case DECODE_STATE_Q:
            {
                 if (token == TOKEN_VALUE)
                {
                    value_i64 = value + (value_i64 * 10);
                }
                else
                {
                    value_q = negative ? -value_i64 : value_i64;
                    negative = false;
                    value_i64 = 0;
                    negative = false;

                    if (!AddValue((double)value_i, (double)value_q))
                    {
                        state = DECODE_STATE_ERROR;
                    }
                    else  if (token == TOKEN_WHITESPACE || token == TOKEN_COMMA)
                    {
                        state = DECODE_STATE_IGNORE;
                    }
                    else if (token == TOKEN_NEWLINE)
                    {
                        state = DECODE_STATE_IDLE;
                    }
                    else
                    {
                        state = DECODE_STATE_ERROR;
                    }
                }
                break;
            }
            case DECODE_STATE_IGNORE:
            {
                if (token == TOKEN_NEWLINE)
                {
                    state = DECODE_STATE_IDLE;
                }
                break;
            }
        }
    }

    return state != DECODE_STATE_ERROR;
}

bool IQData::ReadFile(FILE *file)
{
/*    char buffer[1024];

    while (fgets(buffer, sizeof(buffer) - 1, file) == EOF)
    {

    }*/

    return true;
}