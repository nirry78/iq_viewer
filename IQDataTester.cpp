#include "IQDataTester.h"
#include <stdio.h>

IQDataTester::IQDataTester():
    m_IQData(NULL),
    m_OutputFile(NULL)
{

}

IQDataTester::~IQDataTester()
{
    if (m_IQData)
    {
        delete m_IQData;
        m_IQData = NULL;
    }
}

int IQDataTester::GenerateData()
{
    int res = EXIT_SUCCESS;

    m_IQData = new IQData(128);
    if (m_IQData)
    {
        for (size_t index = 0; index < 512; index++)
        {
            double i = 240.0 * sin((double)index / 90.0 * M_PI);
            double q = 240.0 * cos((double)index / 90.0 * M_PI);
            m_IQData->AddValue(i, q);
        }

        if (m_OutputFile)
        {
            FILE *f = fopen(m_OutputFile, "wt");
            if (f)
            {
                m_IQData->Dump(f);
                fclose(f);
            }
        }
    }

    return res;
}

int IQDataTester::Run(int argc, char** argv)
{
    int res = EXIT_SUCCESS;
    bool generateData = false;

    for (int index = 1; index < argc; index++)
    {
        const char *option = argv[index];

        if (option[0] == '-' || option[0] == '/')
        {
            switch (option[1])
            {
                case 'h':
                {
                    break;
                }
                case 'g':
                {
                    generateData = true;
                    break;
                }
                case 'o':
                {
                    if (index + 1 < argc)
                    {
                        m_OutputFile = argv[index + 1];
                        index++;
                    }
                    break;
                }
            }
        }
    }

    if (generateData)
    {
        res = GenerateData();
    }

    return res;
}

int main(int argc, char** argv)
{
    IQDataTester *iqDataTester = new IQDataTester();
    int res = EXIT_FAILURE;
    if (iqDataTester)
    {
        res = iqDataTester->Run(argc, argv);
        delete iqDataTester;
    }
    return res;
}
