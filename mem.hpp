#ifndef MEM_HPP
#define MEM_HPP

typedef unsigned char BYTE;
typedef short int WORD;
typedef unsigned short int UWORD;

class ULA_Mem0
{
public:
    virtual void writeByte (const UWORD address, const BYTE val ) = 0; 
    virtual void writeAttrib (const UWORD address, const BYTE val ) = 0; 
    virtual BYTE readByteIO( const UWORD address ) = 0;
};

class MEM
{
    friend class Monitor;

public:
    BYTE readByte( const UWORD address ) const;
    WORD readWord( const UWORD address ) const;

    void writeByte( const UWORD address, const BYTE val  );
    void writeWord( const UWORD address, const WORD val  );

    BYTE readByteIO( const UWORD address ) const;
    void writeByteIO( const UWORD address, const BYTE val  );

// temp
    
//private:
    BYTE mM[0x10000];   // just 64KB
    BYTE mIO_out[0x10000];   // just 64KB
    ULA_Mem0* pULA;

};

#define MAKE_WORD(a,b) (((WORD)a << 8) + b)
inline
void SPLIT_WORD(const WORD w, BYTE& h, BYTE& l)
{
    l = (BYTE) w;
    h = (BYTE) ( w >> 8 );
}
#endif // Z80_CPU_HPP

