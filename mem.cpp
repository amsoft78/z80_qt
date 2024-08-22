#include "mem.hpp"


BYTE MEM::readByte( const UWORD address ) const
{
    return mM[address];
}

// LITTLE ENDIAN
WORD MEM::readWord( const UWORD address ) const
{
    WORD r = 0;
    r =  mM[address];
    r = r | (mM[(UWORD)(address+1)] << 8 );
    return r;
}

BYTE MEM::readByteIO( const UWORD address ) const
{
    return pULA->readByteIO(address);
}

void MEM::writeByteIO( const UWORD address, const BYTE val  )
{
    mIO_out[address] = val;
    // simulation
    if( address == 0xFE )
        pULA->writeByte( 0x4000, val );
}

void MEM::writeByte( const UWORD address, const BYTE val  )
{
    UWORD uaddr = address;
    // ROM
    if( uaddr < 0x4000 )
        return;
    mM[uaddr] = val;
    // ULA
    if( uaddr <= 0x57FF)
        pULA->writeByte ( address, val);
    else
    if( uaddr <= 0x5AFF)
        pULA->writeAttrib ( address, val);
    
}

void MEM::writeWord( const UWORD address, const WORD val  )
{
    mM[address] = (BYTE) val;
    mM[(UWORD)(address+1)] = (BYTE) ( val >> 8 );

}


