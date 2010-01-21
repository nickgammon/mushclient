
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
class MString : public CString
{

        int             length, last;
        const int     maxlength;
        bool    alternate;
        CString primary, secondary;

public:

        MString();
        MString(const char*, const int max);
        MString(const CString&, const int max);
        bool SlavoGermanic();
        bool IsVowel(int at);
        inline void MetaphAdd(const char* main);
        inline void MetaphAdd(const char* main, const char* alt);
        bool StringAt(int start, int length, ... );
        void DoubleMetaphone(CString &metaph, CString &metaph2);

};

