#ifndef _IQ_DATA_TESTER_H
#define _IQ_DATA_TESTER_H

#include "Platform.h"
#include "IQData.h"

class IQDataTester
{
    private:
        IQData *m_IQData;
        const char *m_OutputFile;

        int GenerateData();
    public:
        IQDataTester();
        virtual ~IQDataTester();

        int Run(int argc, char** argv);
};

#endif /* _IQ_DATA_TESTER_H */