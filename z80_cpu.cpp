#include "z80_cpu.hpp"
#include <stdlib.h>
#include <iostream>


Z80_CPU* Monitor::pCPU = NULL;
bool Monitor::m_bOn = true;

void Monitor::step( const char* sLabel, const int s )
{

    if( !m_bOn && ( sLabel[0] != 'E' || sLabel[1] != 'R' ) )
        return;

    std::cout << std::hex << pCPU->M1_PC << " ";
    UWORD addr = (UWORD) pCPU->M1_PC;
    std::cout << sLabel;
    for( int i=0; i<s; i++ )
    {
        BYTE b = pCPU->m_pMem->mM[addr++];
        std::cout <<  " " << (WORD) b;
    }
    std::cout << std::endl;

    BYTE b0 = pCPU->m_pMem->mM[pCPU->M1_PC];
    if( b0 == 0xCB || 
        b0 == 0xDD ||
        b0 == 0xED ||
        b0 == 0xFD)
    {
        UWORD w = pCPU->m_pMem->readWord(pCPU->M1_PC);
        if(! m_bTable16[w] )
        {
            dump();
            m_bTable16[w] = true;
        }
    }
    else
    {
        if(! m_bTable8[b0] )
        {
            dump();
            m_bTable8[b0] = true;
        }
    }
}

void Monitor::dump()
{

    std::cout << std::hex;
    std::cout << "A ";
    std::cout << (WORD) pCPU->A << std::endl;
    std::cout << "BC ";
    std::cout << (WORD) pCPU->B << "." << (WORD) pCPU->C << std::endl;
    std::cout << "DE ";
    std::cout << (WORD) pCPU->D << "." << (WORD) pCPU->E << std::endl;
    std::cout << "HL ";
    std::cout << (WORD) pCPU->H << "." << (WORD) pCPU->L << std::endl;
    std::cout << "I ";
    std::cout << (WORD) pCPU->I << std::endl;

    std::cout << "PC " << pCPU->PC << std::endl;
    std::cout << "SP " << pCPU->SP << std::endl;

    std::cout << "I_X " << pCPU->IX << std::endl;
    std::cout << "IY " << pCPU->IY << std::endl;

    std::cout << std::endl << "Flags: " << std::endl;
    std::cout << "C "<< pCPU->fl_get_C() << std::endl;
    std::cout << "N "<< pCPU->fl_get_N() << std::endl;
    std::cout << "PV "<< pCPU->fl_get_PV() << std::endl;
    std::cout << "H "<< pCPU->fl_get_H() << std::endl;
    std::cout << "Z "<< pCPU->fl_get_Z() << std::endl;
    std::cout << "S "<< pCPU->fl_get_S() << std::endl;
    std::cout << "-----------------------------" << std::endl;
}

void Monitor::reset()
{
    std::cerr << "Monitor::reset()"<< std::endl;
    pCPU->setreset();
}

bool Monitor::m_bTable8[0x100];
bool Monitor::m_bTable16[0x10000];

void Monitor::clearTables()
{
    for( int i=0; i< 0x100; i++ )
        m_bTable8[i]=false;
    for( int i=0; i< 0x10000; i++ )
        m_bTable16[i]=false;
}

Z80_CPU::Z80_CPU( MEM* pMem)
{
    m_pMem = pMem;
    m_bEI = false;
    m_bInterrupt = false;
    m_bReset = false;
    m_bHalt = false;
}

void Z80_CPU::reset(UWORD nPC)
{
    PC = nPC;
    SP = 0x0000;

//    A = 0x00;
//    F = 0x00;
    
//    B = 0x00;
//    C = 0x00;

//    D = 0x00;
//    E = 0x00;

//    H = 0x00;
//    L = 0x00;

//    IX = 0x0000;
    IY = 0x0000;

    I = 0x00; // INTERRUPT vector

    R = 0x00; // refresh register
/*
    // shadows - prim registers
    A_PR = 0x00;
    F_PR = 0x00;
    
    B_PR = 0x00;
    C_PR = 0x00;

    D_PR = 0x00;
    E_PR = 0x00;

    H_PR = 0x00;
    L_PR = 0x00;
*/
}

void Z80_CPU::interrupt()
{
    m_bInterrupt = true;
}

void Z80_CPU::setreset()
{
    m_bReset = true;
}

void Z80_CPU::executionLoop()
{
    int res = 0;
    while( res == 0 )
    {
        M1_PC = PC;
        if( !m_bHalt )
            res = fetch();
        
        if( m_bReset )
        {
            m_bHalt = false;
            m_bReset = false;
            M_RST_00();
            continue;
        }
        if( m_bInterrupt )
        {
            if( m_bEI )
            {
                m_bHalt = false;
                m_bEI = false;
                // accept INT
                M_RST_38();
            }
            m_bInterrupt = false;
        }
    }
}

int Z80_CPU::fetch()
{
    // read first byte (mnemonic)
    BYTE b1 = m_pMem->readByte( PC );
    PC++;

    // decode

    switch( b1 )
    {
        case 0xCB:
            if( handle_prefix_CB() )
                return -1;
            break;
        case 0xDD:
            if( handle_prefix_DD() )
                return -1;
            break;
        case 0xED:
            if( handle_prefix_ED() )
                return -1;
            break;
        case 0xFD:
            if( handle_prefix_FD() )
                return -1;
            break;
        case 0x8E: // ADC A,(HL)
            M_ADC_A_OF_HL();
            Monitor::step("ADC A, (HL)", 1);
            break;
        case 0x8F: // ADC A, A
            M_ADC_A_A();
            Monitor::step("ADC A, A", 1);
            break;
        case 0x88: // ADC A, B
            M_ADC_A_B();
            Monitor::step("ADC A, B", 1);
            break;
        case 0x89: // ADC A, C
            M_ADC_A_C();
            Monitor::step("ADC A, C", 1);
            break;
        case 0x8A: // ADC A, D
            M_ADC_A_D();
            Monitor::step("ADC A, D", 1);
            break;
        case 0x8B: // ADC A, E
            M_ADC_A_E();
            Monitor::step("ADC A, E", 1);
            break;
        case 0x8C: // ADC A, H
            M_ADC_A_H();
            Monitor::step("ADC A, H", 1);
            break;
        case 0x8D: // ADC A, L
            M_ADC_A_L();
            Monitor::step("ADC A, L", 1);
            break;
        case 0xCE: // ADC A,x
            M_ADC_A_x();
            Monitor::step("ADC A, x", 2);
            break;            
        case 0x86: // ADD A,(HL)
            M_ADD_A_OF_HL();
            Monitor::step("ADD A, (HL)", 1);
            break;
        case 0x87: // ADD A,A
            M_ADD_A_A();
            Monitor::step("ADD A, A", 1);
            break;
        case 0x80: // ADD A,B
            M_ADD_A_B();
            Monitor::step("ADD A, B", 1);
            break;
        case 0x81: // ADD A,C
            M_ADD_A_C();
            Monitor::step("ADD A, C", 1);
            break;
        case 0x82: // ADD A,D
            M_ADD_A_D();
            Monitor::step("ADD A, D", 1);
            break;
        case 0x83: // ADD A,E
            M_ADD_A_E();
            Monitor::step("ADD A, E", 1);
            break;
        case 0x84: // ADD A,H
            M_ADD_A_H();
            Monitor::step("ADD A, H", 1);
            break;
        case 0x85: // ADD A,L
            M_ADD_A_L();
            Monitor::step("ADD A, L", 1);
            break;
        case 0xC6: // ADD A,x
            M_ADD_A_x();
            Monitor::step("ADD A, x", 2);
            break;            
        case 0x09: // ADD HL,BC
            M_ADD_HL_BC();
            Monitor::step("ADD HL, BC", 1);
            break;            
        case 0x19: // ADD HL,DE
            M_ADD_HL_DE();
            Monitor::step("ADD HL, DE", 1);
            break;            
        case 0x29: // ADD HL,HL
            M_ADD_HL_HL();
            Monitor::step("ADD HL, HL", 1);
            break;            
        case 0x39: // ADD HL,SP
            M_ADD_HL_SP();
            Monitor::step("ADD HL, SP", 1);
            break;   
        case 0xA6:
            M_AND_OF_HL();  // AND (HL)
            Monitor::step("AND (HL)", 1);
            break;
        case 0xA7:
            M_AND_A();  // AND A
            Monitor::step("AND A", 1);
            break;
        case 0xA0:
            M_AND_B();  // AND B
            Monitor::step("AND B", 1);
            break;
        case 0xA1:
            M_AND_C();  // AND C
            Monitor::step("AND C", 1);
            break;
        case 0xA2:
            M_AND_D();  // AND D
            Monitor::step("AND D", 1);
            break;
        case 0xA3:
            M_AND_E();  // AND E
            Monitor::step("AND E", 1);
            break;
        case 0xA4:
            M_AND_H();  // AND H
            Monitor::step("AND H", 1);
            break;
        case 0xA5:
            M_AND_L();  // AND L
            Monitor::step("AND L", 1);
            break;
        case 0xE6:
            M_AND_x();  // AND x
            Monitor::step("AND x", 2);
            break;
        case 0xCD:
            M_CALL();
            Monitor::step("CALL xx", 3);
            break;
        case 0xDC:
            M_CALL_C();
            Monitor::step("CALL C, xx", 3);
            break;
        case 0xFC:
            M_CALL_M();
            Monitor::step("CALL M, xx", 3);
            break;
        case 0xD4:
            M_CALL_NC();
            Monitor::step("CALL NC, xx", 3);
            break;
        case 0xC4:
            M_CALL_NZ();
            Monitor::step("CALL NZ, xx", 3);
            break;
        case 0xF4:
            M_CALL_P();
            Monitor::step("CALL P, xx", 3);
            break;
        case 0xEC:
            M_CALL_PE();
            Monitor::step("CALL PE, xx", 3);
            break;
        case 0xE4:
            M_CALL_PO();
            Monitor::step("CALL PO, xx", 3);
            break;
        case 0xCC:
            M_CALL_Z();
            Monitor::step("CALL Z, xx", 3);
            break;
        case 0x3F:
            M_CCF();
            Monitor::step("CCF", 1);
            break;
        case 0xBE:
            M_CP_OF_HL();
            Monitor::step("CP (HL)", 1);
            break;
        case 0xBF:
            M_CP_A();
            Monitor::step("CP A", 1);
            break;
        case 0xB8:
            M_CP_B();
            Monitor::step("CP B", 1);
            break;
        case 0xB9:
            M_CP_C();
            Monitor::step("CP C", 1);
            break;
        case 0xBA:
            M_CP_D();
            Monitor::step("CP D", 1);
            break;
        case 0xBB:
            M_CP_E();
            Monitor::step("CP E", 1);
            break;
        case 0xBC:
            M_CP_H();
            Monitor::step("CP H", 1);
            break;
        case 0xBD:
            M_CP_L();
            Monitor::step("CP L", 1);
            break;
        case 0xFE:
            M_CP_x();
            Monitor::step("CP x", 2);
            break;
        case 0x2F:
            M_CPL();
            Monitor::step("CPL", 1);
            break;
        case 0x3D:
            M_DEC_A();
            Monitor::step("DEC A", 1);
            break;
        case 0x05:
            M_DEC_B();
            Monitor::step("DEC B", 1);
            break;
        case 0x0D:
            M_DEC_C();
            Monitor::step("DEC C", 1);
            break;
        case 0x15:
            M_DEC_D();
            Monitor::step("DEC D", 1);
            break;
        case 0x1D:
            M_DEC_E();
            Monitor::step("DEC E", 1);
            break;
        case 0x25:
            M_DEC_H();
            Monitor::step("DEC H", 1);
            break;
        case 0x2D:
            M_DEC_L();
            Monitor::step("DEC L", 1);
            break;
        case 0x0B:
            M_DEC_BC();
            Monitor::step("DEC BC", 1);
            break;
        case 0x1B:
            M_DEC_DE();
            Monitor::step("DEC DE", 1);
            break;
        case 0x2B:
            M_DEC_HL();
            Monitor::step("DEC HL", 1);
            break;
        case 0x3B:
            M_DEC_SP();
            Monitor::step("DEC SP", 1);
            break;
        case 0x35:
            M_DEC_OF_HL();
            Monitor::step("DEC (HL)", 1);
            break;
        case 0x10:
            M_DJNZ();
            Monitor::step("DJNZ e", 2);
            break;
        case 0xF3:
            M_DI();
            Monitor::step("DI", 1);
            break;
        case 0xFB:
            M_EI();
            Monitor::step("EI", 1);
            break;
        case 0xD9:
            M_EXX();
            Monitor::step("EXX", 1);
            break;
        case 0x08:
            M_EX_AF_AFP();
            Monitor::step("EX AF, A'F'", 1);
            break;
        case 0xEB:
            M_EX_DE_HL();
            Monitor::step("EX DE, HL", 1);
            break;
        case 0xE3:
            M_EX_OF_SP_HL();
            Monitor::step("EX (SP), HL", 1);
            break;
        case 0x76:
            M_HALT();
            Monitor::step("HALT", 1);
            break;
        case 0xDB:
            M_IN_A_OF_x();
            Monitor::step("IN A, (x)", 2);
            break;
        case 0x3C:
            M_INC_A();
            Monitor::step("INC A", 1);
            break;
        case 0x04:
            M_INC_B();
            Monitor::step("INC B", 1);
            break;
        case 0x0C:
            M_INC_C();
            Monitor::step("INC C", 1);
            break;
        case 0x14:
            M_INC_D();
            Monitor::step("INC D", 1);
            break;
        case 0x1C:
            M_INC_E();
            Monitor::step("INC E", 1);
            break;
        case 0x24:
            M_INC_H();
            Monitor::step("INC H", 1);
            break;
        case 0x2C:
            M_INC_L();
            Monitor::step("INC L", 1);
            break;
        case 0x03:
            M_INC_BC();
            Monitor::step("INC BC", 1);
            break;
        case 0x13:
            M_INC_DE();
            Monitor::step("INC DE", 1);
            break;
        case 0x23:
            M_INC_HL();
            Monitor::step("INC HL", 1);
            break;
        case 0x033:
            M_INC_SP();
            Monitor::step("INC SP", 1);
            break;
        case 0x34:
            M_INC_OF_HL();
            Monitor::step("INC (HL)", 1);
            break;
        case 0xC3:
            M_JP();
            Monitor::step("JP", 3);
            break;
        case 0xE9:
            M_JP_OF_HL();
            Monitor::step("JP (HL)", 1);
            break;
        case 0xDA:
            M_JP_C();
            Monitor::step("JP C, xx", 3);
            break;
        case 0xFA:
            M_JP_M();
            Monitor::step("JP M, xx", 3);
            break;
        case 0xD2:
            M_JP_NC();
            Monitor::step("JP NC, xx", 3);
            break;
        case 0xC2:
            M_JP_NZ();
            Monitor::step("JP NZ, xx", 3);
            break;
        case 0xF2:
            M_JP_P();
            Monitor::step("JP P, xx", 3);
            break;
        case 0xEA:
            M_JP_PE();
            Monitor::step("JP PE, xx", 3);
            break;
        case 0xE2:
            M_JP_PO();
            Monitor::step("JP PO, xx", 3);
            break;
        case 0xCA:
            M_JP_Z();
            Monitor::step("JP Z, xx", 3);
            break;
        case 0x38:
            M_JR_C();
            Monitor::step("JR C, x", 2);
            break;
        case 0x30:
            M_JR_NC();
            Monitor::step("JR NC, x", 2);
            break;
        case 0x20:
            M_JR_NZ();
            Monitor::step("JR NZ, x", 2);
            break;
        case 0x28:
            M_JR_Z();
            Monitor::step("JR Z, x", 2);
            break;
        case 0x18:
            M_JR_x();
            Monitor::step("JR x", 2);
            break;
        case 0x02:
            M_LD_OF_BC_A();
            Monitor::step("LD (BC), A", 1);
            break;
        case 0x12:
            M_LD_OF_DE_A();
            Monitor::step("LD (DE), A", 1);
            break;
        case 0x77:
            M_LD_OF_HL_A();
            Monitor::step("LD (HL), A", 1);
            break;
        case 0x70:
            M_LD_OF_HL_B();
            Monitor::step("LD (HL), B", 1);
            break;
        case 0x71:
            M_LD_OF_HL_C();
            Monitor::step("LD (HL), C", 1);
            break;
        case 0x72:
            M_LD_OF_HL_D();
            Monitor::step("LD (HL), D", 1);
            break;
        case 0x73:
            M_LD_OF_HL_E();
            Monitor::step("LD (HL), E", 1);
            break;
        case 0x74:
            M_LD_OF_HL_H();
            Monitor::step("LD (HL), H", 1);
            break;
        case 0x75:
            M_LD_OF_HL_L();
            Monitor::step("LD (HL), L", 1);
            break;
        case 0x36:
            M_LD_OF_HL_x();
            Monitor::step("LD (HL), x", 2);
            break;
        case 0x0A:
            M_LD_A_OF_BC();
            Monitor::step("LD A, (BC)", 1);
            break;
        case 0x1A:
            M_LD_A_OF_DE();
            Monitor::step("LD A,(DE)", 1);
            break;
        case 0x7E:
            M_LD_A_OF_HL();
            Monitor::step("LD A,(HL)", 1);
            break;
        case 0x7F:
            //M_LD_A_A();
            Monitor::step("LD A, A", 1);
            break;
        case 0x78:
            M_LD_A_B();
            Monitor::step("LD A, B", 1);
            break;
        case 0x79:
            M_LD_A_C();
            Monitor::step("LD A,C", 1);
            break;
        case 0x7A:
            M_LD_A_D();
            Monitor::step("LD A, D", 1);
            break;
        case 0x7B:
            M_LD_A_E();
            Monitor::step("LD A, E", 1);
            break;
        case 0x7C:
            M_LD_A_H();
            Monitor::step("LD A, H", 1);
            break;
        case 0x7D:
            M_LD_A_L();
            Monitor::step("LD A, L", 1);
            break;
        case 0x3A:
            M_LD_A_OF_xx();
            Monitor::step("LD A, (xx)", 3);
            break;
        case 0x3E:
            M_LD_A_x();  // LD A, x
            Monitor::step("LD A, x", 2);
            break;
        case 0x46:
            M_LD_B_OF_HL();
            Monitor::step("LD B, (HL)", 1);
            break;
        case 0x47:
            M_LD_B_A();
            Monitor::step("LD B, A", 1);
            break;
        case 0x40:
            //M_LD_B_B();
            Monitor::step("LD B, B", 1);
            break;
        case 0x41:
            M_LD_B_C();
            Monitor::step("LD B, C", 1);
            break;
        case 0x42:
            M_LD_B_D();
            Monitor::step("LD B, D", 1);
            break;
        case 0x43:
            M_LD_B_E();
            Monitor::step("LD B, E", 1);
            break;
        case 0x44:
            M_LD_B_H();
            Monitor::step("LD B, H", 1);
            break;
        case 0x45:
            M_LD_B_L();
            Monitor::step("LD B, L", 1);
            break;
        case 0x06:
            M_LD_B_x();
            Monitor::step("LD B, x", 2);
            break;
        case 0x4E:
            M_LD_C_OF_HL();
            Monitor::step("LD C, (HL)", 1);
            break;
        case 0x4F:
            M_LD_C_A();
            Monitor::step("LD C, A", 1);
            break;
        case 0x48:
            M_LD_C_B();
            Monitor::step("LD C, B", 1);
            break;
        case 0x49:
            //M_LD_C_C();
            Monitor::step("LD C, C", 1);
            break;
        case 0x4A:
            M_LD_C_D();
            Monitor::step("LD C, D", 1);
            break;
        case 0x4B:
            M_LD_C_E();
            Monitor::step("LD C, E", 1);
            break;
        case 0x4C:
            M_LD_C_H();
            Monitor::step("LD C, H", 1);
            break;
        case 0x4D:
            M_LD_C_L();
            Monitor::step("LD C, L", 1);
            break;
        case 0x0E:
            M_LD_C_x();
            Monitor::step("LD C, x", 2);
            break;
        case 0x56:
            M_LD_D_OF_HL();
            Monitor::step("LD D, (HL)", 1);
            break;
        case 0x57:
            M_LD_D_A();
            Monitor::step("LD D, A", 1);
            break;
        case 0x50:
            M_LD_D_B();
            Monitor::step("LD D, B", 1);
            break;
        case 0x51:
            M_LD_D_C();
            Monitor::step("LD D, C", 1);
            break;
        case 0x52:
            //M_LD_D_D();
            Monitor::step("LD D, D", 1);
            break;
        case 0x53:
            M_LD_D_E();
            Monitor::step("LD D, E", 1);
            break;
        case 0x54:
            M_LD_D_H();
            Monitor::step("LD D, H", 1);
            break;
        case 0x55:
            M_LD_D_L();
            Monitor::step("LD D, L", 1);
            break;
        case 0x16:
            M_LD_D_x();
            Monitor::step("LD D, x", 2);
            break;
        case 0x5E:
            M_LD_E_OF_HL();
            Monitor::step("LD E, (HL)", 1);
            break;
        case 0x5F:
            M_LD_E_A();
            Monitor::step("LD E, A", 1);
            break;
        case 0x58:
            M_LD_E_B();
            Monitor::step("LD E, B", 1);
            break;
        case 0x59:
            M_LD_E_C();
            Monitor::step("LD E, C", 1);
            break;
        case 0x5A:
            M_LD_E_D();
            Monitor::step("LD E, D", 1);
            break;
        case 0x5B:
            //M_LD_E_E();
            Monitor::step("LD E, E", 1);
            break;
        case 0x5C:
            M_LD_E_H();
            Monitor::step("LD E, H", 1);
            break;
        case 0x5D:
            M_LD_E_L();
            Monitor::step("LD E, L", 1);
            break;
        case 0x1E:
            M_LD_E_x();
            Monitor::step("LD E, x", 2);
            break;
        case 0x66:
            M_LD_H_OF_HL();
            Monitor::step("LD H, (HL)", 1);
            break;
        case 0x67:
            M_LD_H_A(); // LD H, A
            Monitor::step("LD H, A", 1);
            break;
        case 0x60:
            M_LD_H_B(); // LD H, B
            Monitor::step("LD H, B", 1);
            break;
        case 0x61:
            M_LD_H_C(); // LD H, C
            Monitor::step("LD H, C", 1);
            break;
        case 0x62:
            M_LD_H_D(); // LD H, D
            Monitor::step("LD H, D", 1);
            break;
        case 0x63:
            M_LD_H_E(); // LD H, E
            Monitor::step("LD H, E", 1);
            break;
        case 0x64:
            // M_LD_H_H(); // LD H, H
            Monitor::step("LD H, H", 1);
            break;
        case 0x65:
            M_LD_H_L();
            Monitor::step("LD H, L", 1);
            break;
        case 0x26:
            M_LD_H_x(); // LD H, x
            Monitor::step("LD H, x", 2);
            break;
        case 0x6E:
            M_LD_L_OF_HL();
            Monitor::step("LD L, (HL)", 3);
            break;
        case 0x6F:
            M_LD_L_A(); // LD L, A
            Monitor::step("LD L, A", 1);
            break;
        case 0x68:
            M_LD_L_B(); // LD L, B
            Monitor::step("LD L, B", 1);
            break;
        case 0x69:
            M_LD_L_C(); // LD L, C
            Monitor::step("LD L, C", 1);
            break;
        case 0x6A:
            M_LD_L_D(); // LD L, D
            Monitor::step("LD L, D", 1);
            break;
        case 0x6B:
            M_LD_L_E(); // LD L, E
            Monitor::step("LD L, E", 1);
            break;
        case 0x6C:
            M_LD_L_H(); // LD L, H
            Monitor::step("LD L, H", 1);
            break;
        case 0x32:
            M_LD_OF_xx_A();
            Monitor::step("LD (xx), A", 3);
            break; 
        case 0x22:
            M_LD_OF_xx_HL();
            Monitor::step("LD (xx), HL", 3);
            break; 
        case 0x2A:
            M_LD_HL_OF_xx();
            Monitor::step("LD HL, (xx)", 3);
            break;
        case 0x6D:
            // M_LD_L_L();
            Monitor::step("LD L, L", 1);
            break;
        case 0x2E:
            M_LD_L_x(); // LD L, x
            Monitor::step("LD L, x", 2);
            break;
        case 0x01:
            M_LD_BC_xx();
            Monitor::step("LD BC, xx", 3);
            break;
        case 0x11:
            M_LD_DE_xx();
            Monitor::step("LD DE, xx", 3);
            break;
        case 0x21:
            M_LD_HL_xx();
            Monitor::step("LD HL, xx", 3);
            break;
        case 0x31:
            M_LD_SP_xx();
            Monitor::step("LD SP, xx", 3);
            break;
        case 0xF9:
            M_LD_SP_HL();
            Monitor::step("LD SP, HL", 1);
            break;
        case 0x00:
            Monitor::step("NOP", 1);
            break;
        case 0xB6:
            M_OR_OF_HL();
            Monitor::step("OR (HL)", 1);
            break;
        case 0xB7:
            M_OR_A();
            Monitor::step("OR A", 1);
            break;
        case 0xB0:
            M_OR_B();
            Monitor::step("OR B", 1);
            break;
        case 0xB1:
            M_OR_C();
            Monitor::step("OR C", 1);
            break;
        case 0xB2:
            M_OR_D();
            Monitor::step("OR D", 1);
            break;
        case 0xB3:
            M_OR_E();
            Monitor::step("OR E", 1);
            break;
        case 0xB4:
            M_OR_H();
            Monitor::step("OR H", 1);
            break;
        case 0xB5:
            M_OR_L();
            Monitor::step("OR L", 1);
            break;
        case 0xf6:
            M_OR_x();
            Monitor::step("OR x", 2);
            break;
        case 0xD3:
            M_OUT_OF_x_A();  // OUT (x), A
            Monitor::step("OUT (x), A", 2);
            break;
        case 0xF1:
            M_POP_AF();
            Monitor::step("POP AF", 1);
            break;
        case 0xC1:
            M_POP_BC();
            Monitor::step("POP BC", 1);
            break;
        case 0xD1:
            M_POP_DE();
            Monitor::step("POP DE", 1);
            break;
        case 0xE1:
            M_POP_HL();
            Monitor::step("POP HL", 1);
            break;
        case 0xC9:
            M_RET();
            Monitor::step("RET", 1);
            break;
        case 0xD8:
            M_RET_C();
            Monitor::step("RET C", 1);
            break;
        case 0xF8:
            M_RET_M();
            Monitor::step("RET M", 1);
            break;
        case 0xD0:
            M_RET_NC();
            Monitor::step("RET NC", 1);
            break;
        case 0xC0:
            M_RET_NZ();
            Monitor::step("RET NZ", 1);
            break;
        case 0xF0:
            M_RET_P();
            Monitor::step("RET P", 1);
            break;
        case 0xE8:
            M_RET_PE();
            Monitor::step("RET PE", 1);
            break;
        case 0xE0:
            M_RET_PO();
            Monitor::step("RET PO", 1);
            break;
        case 0xC8:
            M_RET_Z();
            Monitor::step("RET Z", 1);
            break;
        case 0xC7:
            M_RST_00();
            Monitor::step("RST 00", 1);
            break;
        case 0xCF:
            M_RST_08();
            Monitor::step("RST 08", 1);
            break;
        case 0xD7:
            M_RST_10();
            Monitor::step("RST 10", 1);
            break;
        case 0xDF:
            M_RST_18();
            Monitor::step("RST 18", 1);
            break;
        case 0xE7:
            M_RST_20();
            Monitor::step("RST 20", 1);
            break;
        case 0xEF:
            M_RST_28();
            Monitor::step("RST 28", 1);
            break;
        case 0xF7:
            M_RST_30();
            Monitor::step("RST 30", 1);
            break;
        case 0xFF:
            M_RST_38();
            Monitor::step("RST 38", 1);
            break;
        case 0x17:
            M_RLA();
            Monitor::step("RLA", 1);
            break;
        case 0x07:
            M_RLCA();
            Monitor::step("RLCA", 1);
            break;
        case 0xF5:
            M_PUSH_AF();
            Monitor::step("PUSH AF", 1);
            break;
        case 0xC5:
            M_PUSH_BC();
            Monitor::step("PUSH BC", 1);
            break;
        case 0xD5:
            M_PUSH_DE();
            Monitor::step("PUSH DE", 1);
            break;
        case 0xE5:
            M_PUSH_HL();
            Monitor::step("PUSH HL", 1);
            break;
        case 0x9E:
            M_SBC_OF_HL();
            Monitor::step("SBC (HL)", 1);
            break;
        case 0x9F:
            M_SBC_A();
            Monitor::step("SBC A", 1);
            break;
        case 0x98:
            M_SBC_B();
            Monitor::step("SBC B", 1);
            break;
        case 0x99:
            M_SBC_C();
            Monitor::step("SBC C", 1);
            break;
        case 0x9A:
            M_SBC_D();
            Monitor::step("SBC D", 1);
            break;
        case 0x9B:
            M_SBC_E();
            Monitor::step("SBC E", 1);
            break;
        case 0x9C:
            M_SBC_H();
            Monitor::step("SBC H", 1);
            break;
        case 0x9D:
            M_SBC_L();
            Monitor::step("SBC L", 1);
            break;
        case 0xDE:
            M_SBC_x();
            Monitor::step("SBC x", 1);
            break;
        case 0x37:
            M_SCF();
            Monitor::step("SCF", 1);
            break;
        case 0x96:
            M_SUB_OF_HL();
            Monitor::step("SUB (HL)", 1);
            break;
        case 0x97:
            M_SUB_A();
            Monitor::step("SUB A", 1);
            break;
        case 0x90:
            M_SUB_B();
            Monitor::step("SUB B", 1);
            break;
        case 0x91:
            M_SUB_C();
            Monitor::step("SUB C", 1);
            break;
        case 0x92:
            M_SUB_D();
            Monitor::step("SUB D", 1);
            break;
        case 0x93:
            M_SUB_E();
            Monitor::step("SUB E", 1);
            break;
        case 0x94:
            M_SUB_H();
            Monitor::step("SUB H", 1);
            break;
        case 0x95:
            M_SUB_L();
            Monitor::step("SUB L", 1);
            break;
        case 0xD6:
            M_SUB_x();
            Monitor::step("SUB x", 2);
            break;
        case 0x1F:
            M_RRA();
            Monitor::step("RRA", 1);
            break;
        case 0x0F:
            M_RRCA();
            Monitor::step("RRCA", 1);
            break;
        case 0xAE:
            M_XOR_OF_HL();
            Monitor::step("XOR (HL)", 1);
            break;
        case 0xAF:
            M_XOR_A();
            Monitor::step("XOR A", 1);
            break;
        case 0xA8:
            M_XOR_B();
            Monitor::step("XOR B", 1);
            break;
        case 0xA9:
            M_XOR_C();
            Monitor::step("XOR C", 1);
            break;
        case 0xAA:
            M_XOR_D();
            Monitor::step("XOR D", 1);
            break;
        case 0xAB:
            M_XOR_E();
            Monitor::step("XOR E", 1);
            break;
        case 0xAC:
            M_XOR_H();
            Monitor::step("XOR H", 1);
            break;
        case 0xAD:
            M_XOR_L();
            Monitor::step("XOR L", 1);
            break;
        case 0xEE:
            M_XOR_x();
            Monitor::step("XOR x", 1);
            break;
        default:
            Monitor::step("ERR",4);
            Monitor::dump();
            return -1;
    }
    
    return 0;
    
}

int Z80_CPU::handle_prefix_CB()
{
    // read second byte (mnemonic)
    BYTE b2 = m_pMem->readByte( PC );
    PC++;

    if( (b2 & 0xC0) == 0x40 )
    {
        // BIT b,r
        // decode second byte as
        // 0 	1 	← 	b 	→ 	← 	r 	→ 	 
        int regIndx = ( b2 & 0x07 );
        int bitIndx = (b2 & 0x38) >> 3;
        switch ( regIndx )
        {
            case 0:
            M_BIT_B(bitIndx);
            Monitor::step("BIT x, B", 2);
            break;
            case 1:
            M_BIT_C(bitIndx);
            Monitor::step("BIT x, C", 2);
            break;
            case 2:
            M_BIT_D(bitIndx);
            Monitor::step("BIT x, D", 2);
            break;
            case 3:
            M_BIT_E(bitIndx);
            Monitor::step("BIT x, E", 2);
            break;
            case 4:
            M_BIT_H(bitIndx);
            Monitor::step("BIT x, H", 2);
            break;
            case 5:
            M_BIT_L(bitIndx);
            Monitor::step("BIT x, L", 2);
            break;
            case 6:
            M_BIT_OF_HL(bitIndx);
            Monitor::step("BIT x, (HL)", 2);
            break;
            case 7:
            M_BIT_A(bitIndx);
            Monitor::step("BIT x, A", 2);
            break;
        }
    }
    else
    // set operations
    if( (b2 & 0xC0) == 0xC0 )
    {
        // SET b,r
        // decode second byte as
        // 1 	1 	← 	b 	→ 	← 	r 	→ 	 
        int regIndx = ( b2 & 0x07 );
        int bitIndx = (b2 & 0x38) >> 3;
        switch ( regIndx )
        {
            case 0:
            M_SET_B(bitIndx);
            Monitor::step("SET x, B", 2);
            break;
            case 1:
            M_SET_C(bitIndx);
            Monitor::step("SET x, C", 2);
            break;
            case 2:
            M_SET_D(bitIndx);
            Monitor::step("SET x, D", 2);
            break;
            case 3:
            M_SET_E(bitIndx);
            Monitor::step("SET x, E", 2);
            break;
            case 4:
            M_SET_H(bitIndx);
            Monitor::step("SET x, H", 2);
            break;
            case 5:
            M_SET_L(bitIndx);
            Monitor::step("SET x, L", 2);
            break;
            case 6:
            M_SET_OF_HL(bitIndx);
            Monitor::step("SET x, (HL)", 2);
            break;
            case 7:
            M_SET_A(bitIndx);
            Monitor::step("SET x, A", 2);
            break;
        }
    }
    else
    // reset operations
    if( (b2 & 0xC0) == 0x80 )
    {
        // RESET b,r
        // decode second byte as
        // 1 	0 	← 	b 	→ 	← 	r 	→ 	 
        int regIndx = ( b2 & 0x07 );
        int bitIndx = (b2 & 0x38) >> 3;
        switch ( regIndx )
        {
            case 0:
            M_RES_B(bitIndx);
            Monitor::step("RES x, B", 2);
            break;
            case 1:
            M_RES_C(bitIndx);
            Monitor::step("RES x, C", 2);
            break;
            case 2:
            M_RES_D(bitIndx);
            Monitor::step("RES x, D", 2);
            break;
            case 3:
            M_RES_E(bitIndx);
            Monitor::step("RES x, E", 2);
            break;
            case 4:
            M_RES_H(bitIndx);
            Monitor::step("RES x, H", 2);
            break;
            case 5:
            M_RES_L(bitIndx);
            Monitor::step("RES x, L", 2);
            break;
            case 6:
            M_RES_OF_HL(bitIndx);
            Monitor::step("RES x, (HL)", 2);
            break;
            case 7:
            M_RES_A(bitIndx);
            Monitor::step("RES x, A", 2);
            break;
        }
    }   
    else
    // RL operations
    if( (b2 & 0xF8) == 0x10 )
    {
        switch( b2 )
        {
            case 0x16:
            M_RL_OF_HL();
            Monitor::step("RL (HL)" , 2);
            break;
            case 0x17:
            M_RL_A();
            Monitor::step("RL A" , 2);
            break;
            case 0x10:
            M_RL_B();
            Monitor::step("RL B" , 2);
            break;
            case 0x11:
            M_RL_C();
            Monitor::step("RL C" , 2);
            break;
            case 0x12:
            M_RL_D();
            Monitor::step("RL D" , 2);
            break;
            case 0x13:
            M_RL_E();
            Monitor::step("RL E" , 2);
            break;
            case 0x14:
            M_RL_H();
            Monitor::step("RL H" , 2);
            break;
            case 0x15:
            M_RL_L();
            Monitor::step("RL L" , 2);
            break;
            default:
            Monitor::step("ERR CB", 4);
            Monitor::dump();
            return -1;
        }
    }
    else
    // RLC operations
    if( (b2 & 0xF8) == 0x00 )
    {
        switch( b2 )
        {
            case 0x06:
            M_RLC_OF_HL();
            Monitor::step("RLC (HL)" , 2);
            break;
            case 0x07:
            M_RLC_A();
            Monitor::step("RLC A" , 2);
            break;
            case 0x00:
            M_RLC_B();
            Monitor::step("RLC B" , 2);
            break;
            case 0x01:
            M_RLC_C();
            Monitor::step("RLC C" , 2);
            break;
            case 0x02:
            M_RLC_D();
            Monitor::step("RLC D" , 2);
            break;
            case 0x03:
            M_RLC_E();
            Monitor::step("RLC E" , 2);
            break;
            case 0x04:
            M_RLC_H();
            Monitor::step("RLC H" , 2);
            break;
            case 0x05:
            M_RLC_L();
            Monitor::step("RLC L" , 2);
            break;
            default:
            Monitor::step("ERR CB", 4);
            Monitor::dump();
            return -1;
        }
    }
    else
    // RR operations
    if( (b2 & 0xF8) == 0x18 )
    {
        switch( b2 )
        {
            case 0x1E:
            M_RR_OF_HL();
            Monitor::step("RR (HL)" , 2);
            break;
            case 0x1F:
            M_RR_A();
            Monitor::step("RR A" , 2);
            break;
            case 0x18:
            M_RR_B();
            Monitor::step("RR B" , 2);
            break;
            case 0x19:
            M_RR_C();
            Monitor::step("RR C" , 2);
            break;
            case 0x1A:
            M_RR_D();
            Monitor::step("RR D" , 2);
            break;
            case 0x1B:
            M_RR_E();
            Monitor::step("RR E" , 2);
            break;
            case 0x1C:
            M_RR_H();
            Monitor::step("RR H" , 2);
            break;
            case 0x1D:
            M_RR_L();
            Monitor::step("RR L" , 2);
            break;
            default:
            Monitor::step("ERR CB", 4);
            Monitor::dump();
            return -1;
        }
    }
    else
    // SRL operations
    if( (b2 & 0xF8) == 0x38 )
    {
        switch( b2 )
        {
            case 0x3E:
            M_SRL_OF_HL();
            Monitor::step("SRL (HL)" , 2);
            break;
            case 0x3F:
            M_SRL_A();
            Monitor::step("SRL A" , 2);
            break;
            case 0x38:
            M_SRL_B();
            Monitor::step("SRL B" , 2);
            break;
            case 0x39:
            M_SRL_C();
            Monitor::step("SRL C" , 2);
            break;
            case 0x3A:
            M_SRL_D();
            Monitor::step("SRL D" , 2);
            break;
            case 0x3B:
            M_SRL_E();
            Monitor::step("SRL E" , 2);
            break;
            case 0x3C:
            M_SRL_H();
            Monitor::step("SRL H" , 2);
            break;
            case 0x3D:
            M_SRL_L();
            Monitor::step("SRL L" , 2);
            break;
            default:
            Monitor::step("ERR CB", 4);
            Monitor::dump();
            return -1;
        }
    }
    else
    {
        Monitor::step("ERR CB", 4);
        Monitor::dump();
        return -1;
    }
    return 0;
}

int Z80_CPU::handle_prefix_DD()
{
    // read second byte (mnemonic)
    BYTE b2 = m_pMem->readByte( PC );
    PC++;

    // decode

    switch( b2 )
    {
        case 0x8E:
            M_ADC_A_OF_IX();
            Monitor::step("ADC A, (IX+d)", 3);
            break;
        case 0x86:
            M_ADD_A_OF_IX();
            Monitor::step("ADD A, (IX+x)", 3);
            break;
        case 0x09:
            M_ADD_IX_BC();
            Monitor::step("ADD IX, BC", 2);
            break;
        case 0xA6:
            M_AND_OF_IX();
            Monitor::step("AND (IX+x)", 3);
            break;
        case 0xBE:
            M_CP_OF_IX();
            Monitor::step("CP (IX+d)", 1);
            break;
        case 0x35:
            M_DEC_OF_IX();
            Monitor::step("DEC (IX+d)", 3);
            break;
        case 0xE3:
            M_EX_OF_SP_IX();
            Monitor::step("EX (SP), IX", 2);
            break;
        case 0x23:
            M_INC_IX();
            Monitor::step("INC IX", 2);
            break;
        case 0x34:
            M_INC_OF_IX();
            Monitor::step("INC (IX+d)", 3);
            break;
        case 0xE9:
            M_JP_OF_IX();
            Monitor::step("JP (IX)", 2);
            break;
        case 0x77:
            M_LD_OF_IX_A();
            Monitor::step("LD (IX+x), A", 3);
            break;
        case 0x70:
            M_LD_OF_IX_B();
            Monitor::step("LD (IX+x), B", 3);
            break;
        case 0x71:
            M_LD_OF_IX_C();
            Monitor::step("LD (IX+x), C", 3);
            break;
        case 0x72:
            M_LD_OF_IX_D();
            Monitor::step("LD (IX+x), D", 3);
            break;
        case 0x73:
            M_LD_OF_IX_E();
            Monitor::step("LD (IX+x), E", 3);
            break;
        case 0x74:
            M_LD_OF_IX_H();
            Monitor::step("LD (IX+x), H", 3);
            break;
        case 0x75:
            M_LD_OF_IX_L();
            Monitor::step("LD (IX+x), L", 3);
            break;
        case 0x36:
            M_LD_OF_IX_x();
            Monitor::step("LD (IX+d), x", 4);
            break;
        case 0x7E:
            M_LD_A_OF_IX();
            Monitor::step("LD A, (IX+d)", 3);
            break;
        case 0x46:
            M_LD_B_OF_IX();
            Monitor::step("LD B, (IX+d)", 3);
            break;
        case 0x4E:
            M_LD_C_OF_IX();
            Monitor::step("LD C, (IX+d)", 3);
            break;
        case 0x56:
            M_LD_D_OF_IX();
            Monitor::step("LD D, (IX+d)", 3);
            break;
        case 0x5E:
            M_LD_E_OF_IX();
            Monitor::step("LD E, (IX+d)", 3);
            break;
        case 0x66:
            M_LD_H_OF_IX();
            Monitor::step("LD H, (IX+d)", 3);
            break;
        case 0x6E:
            M_LD_L_OF_IX();
            Monitor::step("LD L, (IX+d)", 3);
            break;
        case 0xF9:
            M_LD_SP_IX();
            Monitor::step("LD SP, IX", 2);
            break;
        case 0x21:
            M_LD_IX_xx();
            Monitor::step("LD IX, xx", 4);
            break;
        case 0xE1:
            M_POP_IX();
            Monitor::step("POP IX", 2);
            break;
        case 0xE5:
            M_PUSH_IX();
            Monitor::step("PUSH IX", 2);
            break;
        case 0x96:
            M_SUB_OF_IX();
            Monitor::step("SUB (IX+d)", 1);
            break;
        case 0xCB:
            if( bit_op_IX() )
                return -1;
            break;
        default:
            Monitor::step("ERR DD ",4);
            Monitor::dump();
            return -1;
    }

    return 0;
}

int Z80_CPU::handle_prefix_ED()
{
    // read second byte (mnemonic)
    BYTE b2 = m_pMem->readByte( PC );
    PC++;

    // decode

    switch( b2 )
    {
        case 0x44:
            M_NEG();
            Monitor::step("NEG", 2);
            break;
        case 0x78:
            M_IN_A_OF_C();
            Monitor::step("IN A, (C)", 2);
            break;
        case 0x46:
            M_IM(0);
            Monitor::step("IM 0", 2);
            break;
        case 0x56:
            M_IM(1);
            Monitor::step("IM 1", 2);
            break;
        case 0x5E:
            M_IM(2);
            Monitor::step("IM 2", 2);
            break;
        case 0x57:
            M_LD_A_I();
            Monitor::step("LD A, I", 2);
            break;  
        case 0x5F:
            M_LD_A_R();
            Monitor::step("LD A, R", 2);
            break;  
        case 0x47:
            M_LD_I_A();
            Monitor::step("LD I, A", 2);
            break;  
        case 0x43:
            M_LD_OF_xx_BC();
            Monitor::step("LD (xx), BC", 4);
            break;  
        case 0x53:
            M_LD_OF_xx_DE();
            Monitor::step("LD (xx), DE", 4);
            break;  
        case 0x23:
            M_LD_OF_xx_HL();
            Monitor::step("LD (xx), HL", 4);
            break;  
        case 0x73:
            M_LD_OF_xx_SP();
            Monitor::step("LD (xx), SP", 4);
            break;
        case 0x4B:
            M_LD_BC_OF_xx();
            Monitor::step("LD BC, (xx)", 4);
            break;  
        case 0x5B:
            M_LD_DE_OF_xx();
            Monitor::step("LD DE, (xx)", 4);
            break;  
        case 0x7B:
            M_LD_SP_OF_xx();
            Monitor::step("LD SP, (xx)", 4);
            break;  
        case 0xB8:
            M_LDDR();
            Monitor::step("LDDR", 2);
            break;
        case 0xB0:
            M_LDIR();
            Monitor::step("LDIR", 2);
            break;
        case 0x42:
            M_SBC_HL_BC();
            Monitor::step("SBC HL, BC", 2);
            break;  
        case 0x52:
            M_SBC_HL_DE();
            Monitor::step("SBC HL, DE", 2);
            break;  
        case 0x62:
            M_SBC_HL_HL();
            Monitor::step("SBC HL, HL", 2);
            break;  
        case 0x72:
            M_SBC_HL_SP();
            Monitor::step("SBC HL, SP", 2);
            break;  
        default:
            Monitor::step("ERR ED ",4);
            Monitor::dump();
            return -1;
    }

    return 0;
}

int Z80_CPU::handle_prefix_FD()
{
    // read second byte (mnemonic)
    BYTE b2 = m_pMem->readByte( PC );
    PC++;

    // decode

    switch( b2 )
    {
         case 0x8E:
            M_ADC_A_OF_IY();
            Monitor::step("ADC A, (IY+d)", 3);
            break;  
        case 0x86:
            M_ADD_A_OF_IY();
            Monitor::step("ADD A, (IY+d)", 3);
            break;
        case 0xA6:
            M_AND_OF_IY();
            Monitor::step("AND (IY+d)", 3);
            break;
        case 0xBE:
            M_CP_OF_IY();
            Monitor::step("CP (IY+d)", 1);
            break;
        case 0x35:
            M_DEC_OF_IY();
            Monitor::step("DEC (IY+d)", 3);
            break;
        case 0xE3:
            M_EX_OF_SP_IY();
            Monitor::step("EX (SP), IY", 1);
            break;
        case 0x34:
            M_INC_OF_IY();
            Monitor::step("INC (IY+d)", 3);
            break;
        case 0x23:
            M_INC_IY();
            Monitor::step("INC IY", 2);
            break;
        case 0xE9:
            M_JP_OF_IY();
            Monitor::step("JP (IY)", 2);
            break;
        case 0x77:
            M_LD_OF_IY_A();
            Monitor::step("LD (IY+x), A", 3);
            break;
        case 0x70:
            M_LD_OF_IY_B();
            Monitor::step("LD (IY+x), B", 3);
            break;
        case 0x71:
            M_LD_OF_IY_C();
            Monitor::step("LD (IY+x), C", 3);
            break;
        case 0x72:
            M_LD_OF_IY_D();
            Monitor::step("LD (IY+x), D", 3);
            break;
        case 0x73:
            M_LD_OF_IY_E();
            Monitor::step("LD (IY+x), E", 3);
            break;
        case 0x74:
            M_LD_OF_IY_H();
            Monitor::step("LD (IY+x), H", 3);
            break;
        case 0x75:
            M_LD_OF_IY_L();
            Monitor::step("LD (IY+x), L", 3);
            break;
        case 0x36:
            M_LD_OF_IY_x();
            Monitor::step("LD (IY+d), x", 4);
            break;
        case 0x7E:
            M_LD_A_OF_IY();
            Monitor::step("LD A, (IY+d)", 3);
            break;
        case 0x46:
            M_LD_B_OF_IY();
            Monitor::step("LD B, (IY+d)", 3);
            break;
        case 0x4E:
            M_LD_C_OF_IY();
            Monitor::step("LD C, (IY+d)", 3);
            break;
        case 0x56:
            M_LD_D_OF_IY();
            Monitor::step("LD D, (IY+d)", 3);
            break;
        case 0x5E:
            M_LD_E_OF_IY();
            Monitor::step("LD E, (IY+d)", 3);
            break;
        case 0x66:
            M_LD_H_OF_IY();
            Monitor::step("LD H, (IY+d)", 3);
            break;
        case 0x6E:
            M_LD_L_OF_IY();
            Monitor::step("LD L, (IY+d)", 3);
            break;
        case 0xF9:
            M_LD_SP_IY();
            Monitor::step("LD SP, IY", 2);
            break;
        case 0x21:
            M_LD_IY_xx();
            Monitor::step("LD IY, xx", 4);
            break;
        case 0xE1:
            M_POP_IY();
            Monitor::step("POP IY", 2);
            break;
        case 0xE5:
            M_PUSH_IY();
            Monitor::step("PUSH IY", 2);
            break;
        case 0x96:
            M_SUB_OF_IY();
            Monitor::step("SUB (IY+d)", 1);
            break;
        case 0xCB:
            if (bit_op_IY())
                return -1;
            break;
       default:
            Monitor::step("ERR FD ", 4);
            Monitor::dump();
            return -1;
    }

    return 0;
}


void Z80_CPU::M_ADC_A_OF_HL()
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));
    i_add_withC( A, bArg2 );
}

void Z80_CPU::M_ADC_A_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addrArg2 = i_getOffsetAddr( IX );
    BYTE bArg2 = m_pMem->readByte(addrArg2);
    i_add_withC( A, bArg2 );
}

void Z80_CPU::M_ADC_A_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addrArg2 = i_getOffsetAddr( IY );
    BYTE bArg2 = m_pMem->readByte(addrArg2);
    i_add_withC( A, bArg2 );
}

void Z80_CPU::M_ADC_A_A()
{
    i_add_withC( A, A );
}

void Z80_CPU::M_ADC_A_B()
{
    i_add_withC( A, B );
}

void Z80_CPU::M_ADC_A_C()
{
    i_add_withC( A, C );
}

void Z80_CPU::M_ADC_A_D()
{
    i_add_withC( A, D );
}

void Z80_CPU::M_ADC_A_E()
{
    i_add_withC( A, E );
}

void Z80_CPU::M_ADC_A_H()
{
    i_add_withC( A, H );
}

void Z80_CPU::M_ADC_A_L()
{
    i_add_withC( A, L );
}

void Z80_CPU::M_ADC_A_x()
{
    // read immediate argument
    BYTE bArg2 = m_pMem->readByte(PC);
    PC++;
    i_add_withC( A, bArg2 );
}


void Z80_CPU::M_ADD_A_OF_HL()
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));
    i_add( A, bArg2 );
}

void Z80_CPU::M_ADD_A_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addrArg2 = i_getOffsetAddr( IX );
    BYTE bArg2 = m_pMem->readByte(addrArg2);
    i_add( A, bArg2 );
}

void Z80_CPU::M_ADD_A_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addrArg2 = i_getOffsetAddr( IY );
    BYTE bArg2 = m_pMem->readByte(addrArg2);
    i_add( A, bArg2 );
}

void Z80_CPU::M_ADD_A_A()
{
    i_add( A, A );
}

void Z80_CPU::M_ADD_A_B()
{
    i_add( A, B );
}

void Z80_CPU::M_ADD_A_C()
{
    i_add( A, C );
}

void Z80_CPU::M_ADD_A_D()
{
    i_add( A, D );
}

void Z80_CPU::M_ADD_A_E()
{
    i_add( A, E );
}

void Z80_CPU::M_ADD_A_H()
{
    i_add( A, H );
}

void Z80_CPU::M_ADD_A_L()
{
    i_add( A, L );
}

void Z80_CPU::M_ADD_A_x()
{
    // read immediate argument
    BYTE bArg2 = m_pMem->readByte(PC);
    PC++;
    i_add( A, bArg2 );
}

void Z80_CPU::M_ADD_HL_BC()
{
    i_addW( H, L, B, C ); 
}

void Z80_CPU::M_ADD_HL_DE()
{
    i_addW( H, L, D, E ); 
}

void Z80_CPU::M_ADD_HL_HL()
{
    i_addW( H, L, H, L ); 
}

void Z80_CPU::M_ADD_HL_SP()
{
    i_addW( H, L, (BYTE)(SP >> 8), (BYTE)SP ); 
}
void Z80_CPU::M_ADD_IX_BC()
{
    i_addW( IX, B, C );
}

void Z80_CPU::M_AND_OF_HL()
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));
    i_and( bArg2 );
}

 void Z80_CPU::M_AND_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addrArg2 = i_getOffsetAddr( IX );

    BYTE bArg2 = m_pMem->readByte(addrArg2);
    i_and( bArg2 );
}

 void Z80_CPU::M_AND_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addrArg2 = i_getOffsetAddr( IY );
    BYTE bArg2 = m_pMem->readByte(addrArg2);
    i_and( bArg2 );
}

void Z80_CPU::M_AND_A()
{
    i_and( A );
}

void Z80_CPU::M_AND_B()
{
    i_and( B );
}

void Z80_CPU::M_AND_C()
{
    i_and( C );
}

void Z80_CPU::M_AND_D()
{
    i_and( D );
}

void Z80_CPU::M_AND_E()
{
    i_and( E );
}

void Z80_CPU::M_AND_H()
{
    i_and( H );
}

void Z80_CPU::M_AND_L()
{
    i_and( L );
}

void Z80_CPU::M_AND_x()
{
    // read immediate argument
    BYTE bArg2 = m_pMem->readByte(PC);
    PC++;
    i_and( bArg2 );
}

void Z80_CPU::M_CP_OF_HL()
{
    BYTE bArg1 = A;
    // read immediate argument
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));
    i_sub( bArg1, bArg2 );
}

void Z80_CPU::M_CP_OF_IX()
{
    BYTE bArg1 = A;
    UWORD addr = i_getOffsetAddr( IX );
    BYTE bArg2 = m_pMem->readByte( addr );
    i_sub( bArg1, bArg2 );
}

void Z80_CPU::M_CP_OF_IY()
{
    BYTE bArg1 = A;
    UWORD addr = i_getOffsetAddr( IY );
    BYTE bArg2 = m_pMem->readByte( addr );
    i_sub( bArg1, bArg2 );
}

void Z80_CPU::M_CP_A()
{
    BYTE bArg1 = A;
    i_sub( bArg1, A );
}

void Z80_CPU::M_CP_B()
{
    BYTE bArg1 = A;
    i_sub( bArg1, B );
}

void Z80_CPU::M_CP_C()
{
    BYTE bArg1 = A;
    i_sub( bArg1, C );
}

void Z80_CPU::M_CP_D()
{
    BYTE bArg1 = A;
    i_sub( bArg1, D );
}

void Z80_CPU::M_CP_E()
{
    BYTE bArg1 = A;
    i_sub( bArg1, E );
}

void Z80_CPU::M_CP_H()
{
    BYTE bArg1 = A;
    i_sub( bArg1, H );
}

void Z80_CPU::M_CP_L()
{
    BYTE bArg1 = A;
    i_sub( bArg1, L );
}

void Z80_CPU::M_CP_x()
{
    BYTE bArg1 = A;
    // read immediate argument
    BYTE bArg2 = m_pMem->readByte(PC);
    PC++;
    i_sub( bArg1, bArg2 );
}

void Z80_CPU::M_BIT_B(const int bitIndx)
{
    i_bit( B, bitIndx);
}

void Z80_CPU::M_BIT_C(const int bitIndx)
{
    i_bit( C, bitIndx);
}

void Z80_CPU::M_BIT_D(const int bitIndx)
{
    i_bit( D, bitIndx);
}

void Z80_CPU::M_BIT_E(const int bitIndx)
{
    i_bit( E, bitIndx);
}

void Z80_CPU::M_BIT_H(const int bitIndx)
{
    i_bit( H, bitIndx);
}

void Z80_CPU::M_BIT_L(const int bitIndx)
{
    i_bit( L, bitIndx);
}

void Z80_CPU::M_BIT_A(const int bitIndx)
{
    i_bit( A, bitIndx);
}

void Z80_CPU::M_BIT_OF_HL(const int bitIndx)
{
    // read byte from (HL)
    BYTE val = m_pMem->readByte(MAKE_WORD(H,L));
    i_bit( val, bitIndx);
}

void Z80_CPU::M_CALL()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    // push PC
    i_pushW( PC );
    
    PC = addr;
}

void Z80_CPU::M_CALL_C()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( fl_get_C() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CALL_M()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( fl_get_S() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CALL_NC()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( ! fl_get_C() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CALL_NZ()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( ! fl_get_Z() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CALL_P()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( ! fl_get_S() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CALL_PE()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( fl_get_PV() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CALL_PO()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( ! fl_get_PV() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CALL_Z()
{
    UWORD addr =  m_pMem->readWord(PC);
    PC += 2;
    if( fl_get_Z() )
    {
        // push PC
        i_pushW( PC );
    
        PC = addr;
    }
}

void Z80_CPU::M_CCF()
{
    bool bPrevC = fl_get_C();
    fl_set_H( bPrevC );
    fl_set_C( !bPrevC );

    fl_set_N( false );
}

void Z80_CPU::M_CPL()
{
    A= ~A;
    fl_set_H( true );
    fl_set_N( true );
}

void Z80_CPU::M_DEC_A()
{
    i_dec( A );
}

void Z80_CPU::M_DEC_B()
{
    i_dec( B );
}

void Z80_CPU::M_DEC_C()
{
    i_dec( C );
}

void Z80_CPU::M_DEC_D()
{
    i_dec( D );
}

void Z80_CPU::M_DEC_E()
{
    i_dec( E );
}

void Z80_CPU::M_DEC_H()
{
    i_dec( H );
}

void Z80_CPU::M_DEC_L()
{
    i_dec( L );
}

void Z80_CPU::M_DEC_BC()
{
    WORD w = MAKE_WORD (B, C);
    w--;
    SPLIT_WORD( w, B, C );
}

void Z80_CPU::M_DEC_DE()
{
    WORD w = MAKE_WORD (D, E);
    w--;
    SPLIT_WORD( w, D, E );
}

void Z80_CPU::M_DEC_HL()
{
    WORD w = MAKE_WORD (H, L);
    w--;
    SPLIT_WORD( w, H, L );
}

void Z80_CPU::M_DEC_SP()
{
    SP--;
}

void Z80_CPU::M_DEC_OF_HL()
{
    WORD addr = MAKE_WORD(H,L);
    BYTE bArg = m_pMem->readByte( addr );
    i_dec( bArg );
    m_pMem->writeByte( addr, bArg );
}

void Z80_CPU::M_DEC_OF_IX()
{
    UWORD addr = i_getOffsetAddr( IX );
    BYTE bArg = m_pMem->readByte( addr );
    i_dec( bArg );
    m_pMem->writeByte( addr, bArg );
}

void Z80_CPU::M_DEC_OF_IY()
{
    UWORD addr = i_getOffsetAddr( IY );
    BYTE bArg = m_pMem->readByte( addr );
    i_dec( bArg );
    m_pMem->writeByte( addr, bArg );
}


void Z80_CPU::M_DJNZ()
{
    BYTE e = m_pMem->readByte( PC );
    PC++;
    // dec B
    B--;
    if( B != 0)
        PC = PC + (signed char) e;
}


void Z80_CPU::M_DI()
{
    m_bEI = false;
}

void Z80_CPU::M_EI()
{
    m_bEI = true;
}

void Z80_CPU::M_EX_AF_AFP()
{
    // exchange prim registers A and F
    BYTE A_cp = A;
    BYTE F_cp = F;

    A = A_PR;
    F = F_PR;

    A_PR = A_cp;
    F_PR = F_cp;
}

void Z80_CPU::M_EX_DE_HL()
{
    BYTE D_cp = D;
    BYTE E_cp = E;
    
    D = H;
    E = L;
    
    H = D_cp;
    L = E_cp;
}

void Z80_CPU::M_EXX()
{
    // exchange prim registers

    BYTE B_cp = B;
    BYTE C_cp = C;
    BYTE D_cp = D;
    BYTE E_cp = E;
    BYTE H_cp = H;
    BYTE L_cp = L;

    B = B_PR;
    C = C_PR;
    D = D_PR;
    E = E_PR;
    H = H_PR;
    L = L_PR;

    B_PR = B_cp;
    C_PR = C_cp;
    D_PR = D_cp;
    E_PR = E_cp;
    H_PR = H_cp;
    L_PR = L_cp;
}

void Z80_CPU::M_EX_OF_SP_HL()
{
    WORD w = m_pMem->readWord(SP);
    m_pMem->writeWord( SP, MAKE_WORD(H, L) );
    SPLIT_WORD( w, H, L);
}

void Z80_CPU::M_EX_OF_SP_IX()
{
    WORD w = m_pMem->readWord(SP);
    m_pMem->writeWord( SP, IX );
    IX = w;
}

void Z80_CPU::M_EX_OF_SP_IY()
{
    WORD w = m_pMem->readWord(SP);
    m_pMem->writeWord( SP, IY );
    IY = w;
}

void Z80_CPU::M_HALT()
{
    //    PC--;
    m_bHalt = true;
}

void Z80_CPU::M_IM( int m )
{
    // TODO
}

void Z80_CPU::M_IN_A_OF_x()
{
    BYTE port =  m_pMem->readByte( PC );
    PC++;

    UWORD addr = MAKE_WORD(A, port);

    A = m_pMem->readByteIO( addr );
}

void Z80_CPU::M_IN_A_OF_C()
{
    UWORD addr = MAKE_WORD(B, C);
    BYTE b = m_pMem->readByteIO( addr );
    
    fl_set_S( b & 0x80 );
    fl_set_Z( b == 0 );
    fl_set_H( false );
    fl_set_N( false );
    fl_check_parity( b );    

    A = b;
}

void Z80_CPU::M_INC_A()
{
    i_inc( A );
}

void Z80_CPU::M_INC_B()
{
    i_inc( B );
}

void Z80_CPU::M_INC_C()
{
    i_inc( C );
}

void Z80_CPU::M_INC_D()
{
    i_inc( D );
}

void Z80_CPU::M_INC_E()
{
    i_inc( H );
}

void Z80_CPU::M_INC_H()
{
    i_inc( H );
}

void Z80_CPU::M_INC_L()
{
    i_inc( L );
}

void Z80_CPU::M_INC_BC()
{
    i_incW(B, C);
}

void Z80_CPU::M_INC_DE()
{
    i_incW(D, E);
}

void Z80_CPU::M_INC_HL()
{
    i_incW(H, L);
}

void Z80_CPU::M_INC_SP()
{
    SP++;
}

void Z80_CPU::M_INC_IX()
{
    IX++;
}

void Z80_CPU::M_INC_IY()
{
    IY++;
}

void Z80_CPU::M_INC_OF_HL()
{
    UWORD addr = MAKE_WORD(H,L);
    BYTE bArg = m_pMem->readByte( addr );
    i_inc( bArg );
    m_pMem->writeByte( addr, bArg );
}

void Z80_CPU::M_INC_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    BYTE val = m_pMem->readByte(addr);
    i_inc( val );
    m_pMem->writeByte( addr, val );
}

void Z80_CPU::M_INC_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    BYTE val = m_pMem->readByte(addr);
    i_inc( val );
    m_pMem->writeByte( addr, val );
}


void Z80_CPU::M_JP()
{
    // read 2 immediate arguments
    UWORD addr = m_pMem->readWord(PC);
    PC = addr;    
}

void Z80_CPU::M_JP_OF_HL()
{
    PC = MAKE_WORD(H, L);    
}

void Z80_CPU::M_JP_OF_IX()
{
    PC = IX;    
}

void Z80_CPU::M_JP_OF_IY()
{
    PC = IY;    
}

void Z80_CPU::M_JP_C()
{
    if( fl_get_C() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JP_M()
{
    // check if negative
    if( fl_get_S() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JP_NC()
{
    if( ! fl_get_C() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JP_NZ()
{
    if( ! fl_get_Z() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JP_P()
{
    // check if positive
    if( ! fl_get_S() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JP_PO()
{
    if( ! fl_get_PV() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JP_PE()
{
    if( fl_get_PV() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JP_Z()
{
    if( fl_get_Z() )
    {
        // read 2 byte immediate argument
        UWORD addr = m_pMem->readWord(PC);
        PC = addr;
    }
    else
        PC += 2;
}

void Z80_CPU::M_JR_C ()
{
    // read immediate argument
    signed char offset = (signed char)m_pMem->readByte(PC);
    PC++;
    if( fl_get_C() )
        PC = PC + offset;
}

void Z80_CPU::M_JR_NC ()
{
    // read immediate argument
    signed char offset = (signed char)m_pMem->readByte(PC);
    PC++;
    if( ! fl_get_C() )
        PC = PC + offset;
}

void Z80_CPU::M_JR_NZ ()
{
    // read immediate argument
    signed char offset = (signed char)m_pMem->readByte(PC);
    PC++;
    if( ! fl_get_Z() )
        PC = PC + offset;
}

void Z80_CPU::M_JR_Z ()
{
    // read immediate argument
    signed char offset = (signed char)m_pMem->readByte(PC);
    PC++;
    if( fl_get_Z() )
        PC = PC + offset;
}

void Z80_CPU::M_JR_x ()
{
    // read immediate argument
    signed char offset = (signed char)m_pMem->readByte(PC);
    PC++;
    PC = PC + offset;
}

void Z80_CPU::M_LD_OF_BC_A()
{
    UWORD addr = MAKE_WORD(B,C);
    m_pMem->writeByte( addr, A);
}

void Z80_CPU::M_LD_OF_DE_A()
{
    UWORD addr = MAKE_WORD(D,E);
    m_pMem->writeByte( addr, A);
}

void Z80_CPU::M_LD_OF_HL_A()
{
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, A);
}

void Z80_CPU::M_LD_OF_HL_B()
{
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, B);
}

void Z80_CPU::M_LD_OF_HL_C()
{
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, C);
}

void Z80_CPU::M_LD_OF_HL_D()
{
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, D);
}

void Z80_CPU::M_LD_OF_HL_E()
{
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, E);
}

void Z80_CPU::M_LD_OF_HL_H()
{
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, H);
}

void Z80_CPU::M_LD_OF_HL_L()
{
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, L);
}

void Z80_CPU::M_LD_OF_HL_x()
{
    // read immediate argument
    BYTE bArg2 = m_pMem->readByte(PC);
    PC++;
    UWORD addr = MAKE_WORD(H,L);
    m_pMem->writeByte( addr, bArg2 );
}

void Z80_CPU::M_LD_A_OF_BC()
{
    // read byte from (BC)
    A = m_pMem->readByte(MAKE_WORD(B,C));
}

void Z80_CPU::M_LD_A_OF_DE()
{
    // read byte from (DE)
    A = m_pMem->readByte(MAKE_WORD(D,E));
}

void Z80_CPU::M_LD_A_OF_HL()
{
    // read byte from (HL)
    A = m_pMem->readByte(MAKE_WORD(H,L));
}

void Z80_CPU::M_LD_A_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    A = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_A_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    A = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_A_B()
{
    A = B;
}

void Z80_CPU::M_LD_A_C()
{
    A = C;
}

void Z80_CPU::M_LD_A_D()
{
    A = D;
}

void Z80_CPU::M_LD_A_E()
{
    A = E;
}

void Z80_CPU::M_LD_A_H()
{
    A = H;
}

void Z80_CPU::M_LD_A_I()
{
    A = I;
}

void Z80_CPU::M_LD_A_L()
{
    A = L;
}

void Z80_CPU::M_LD_A_R()
{
    A = R ;
}

void Z80_CPU::M_LD_A_x()
{
    // read immediate argument
    BYTE bArg2 = m_pMem->readByte(PC);
    PC++;
    A = bArg2;
}

void Z80_CPU::M_LD_A_OF_xx()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    A = m_pMem->readByte( addr );
}
 
void Z80_CPU::M_LD_B_OF_HL()
{
    // read byte from (HL)
    B = m_pMem->readByte(MAKE_WORD(H,L));
}

void Z80_CPU::M_LD_B_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    B = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_B_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    B = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_B_A()
{
    B = A;
}

void Z80_CPU::M_LD_B_C()
{
    B = C;
}

void Z80_CPU::M_LD_B_D()
{
    B = D;
}

void Z80_CPU::M_LD_B_E()
{
    B = E;
}

void Z80_CPU::M_LD_B_H()
{
    B = H;
}

void Z80_CPU::M_LD_B_L()
{
    B = L;
}

void Z80_CPU::M_LD_B_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    B = val;
}

void Z80_CPU::M_LD_C_OF_HL()
{
    // read byte from (HL)
    C = m_pMem->readByte(MAKE_WORD(H,L));
}

void Z80_CPU::M_LD_C_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    C = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_C_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    C = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_C_A()
{
    C = A;
}

void Z80_CPU::M_LD_C_B()
{
    C = B;
}

void Z80_CPU::M_LD_C_D()
{
    C = D;
}

void Z80_CPU::M_LD_C_E()
{
    C = E;
}

void Z80_CPU::M_LD_C_H()
{
    C = H;
}

void Z80_CPU::M_LD_C_L()
{
    C = L;
}

void Z80_CPU::M_LD_C_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    C = val;
}

void Z80_CPU::M_LD_D_OF_HL()
{
    // read byte from (HL)
    D = m_pMem->readByte(MAKE_WORD(H,L));
}

void Z80_CPU::M_LD_D_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    D = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_D_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    D = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_D_A()
{
    D = A;
}

void Z80_CPU::M_LD_D_B()
{
    D = B;
}

void Z80_CPU::M_LD_D_C()
{
    D = C;
}

void Z80_CPU::M_LD_D_E()
{
    D = E;
}

void Z80_CPU::M_LD_D_H()
{
    D = H;
}

void Z80_CPU::M_LD_D_L()
{
    D = L;
}

void Z80_CPU::M_LD_D_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    D = val;
}

void Z80_CPU::M_LD_E_OF_HL()
{
    // read byte from (HL)
    E = m_pMem->readByte(MAKE_WORD(H,L));
}

void Z80_CPU::M_LD_E_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    E = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_E_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    E = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_E_A()
{
    E = A;
}

void Z80_CPU::M_LD_E_B()
{
    E = B;
}

void Z80_CPU::M_LD_E_C()
{
    E = C;
}

void Z80_CPU::M_LD_E_D()
{
    E = D;
}

void Z80_CPU::M_LD_E_H()
{
    E = H;
}

void Z80_CPU::M_LD_E_L()
{
    E = L;
}

void Z80_CPU::M_LD_E_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    E = val;
}

void Z80_CPU::M_LD_H_OF_HL()
{
    // read byte from (HL)
    H = m_pMem->readByte(MAKE_WORD(H,L));
}

void Z80_CPU::M_LD_H_OF_IX()
{
    // read byte from (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    H = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_H_OF_IY()
{
    // read byte from (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    H = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_H_A ()
{
    H = A;
}

void Z80_CPU::M_LD_H_B ()
{
    H = B;
}

void Z80_CPU::M_LD_H_C ()
{
    H = C;
}

void Z80_CPU::M_LD_H_D ()
{
    H = D;
}

void Z80_CPU::M_LD_H_E ()
{
    H = E;
}

void Z80_CPU::M_LD_H_L ()
{
    H = L;
}

void Z80_CPU::M_LD_H_x ()
{
    // read immediate argument
    H = m_pMem->readByte(PC);
    PC++;
}

void Z80_CPU::M_LD_L_OF_HL()
{
    // read byte from (HL)
    L = m_pMem->readByte(MAKE_WORD(H,L));
}


void Z80_CPU::M_LD_L_A ()
{
    L = A;
}

void Z80_CPU::M_LD_L_B ()
{
    L = B;
}

void Z80_CPU::M_LD_L_C ()
{
    L = C;
}

void Z80_CPU::M_LD_L_D ()
{
    L = D;
}

void Z80_CPU::M_LD_L_E ()
{
    L = E;
}

void Z80_CPU::M_LD_L_H ()
{
    L = H;
}

void Z80_CPU::M_LD_L_x ()
{
    // read immediate argument
    L = m_pMem->readByte(PC);
    PC++;
}

void Z80_CPU::M_LD_L_OF_IX()
{
    UWORD addr = i_getOffsetAddr(IX);
    L = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_L_OF_IY()
{
    UWORD addr = i_getOffsetAddr(IY);
    L = m_pMem->readByte(addr);
}

void Z80_CPU::M_LD_I_A ()
{
    I = A;
}

void Z80_CPU::M_LD_OF_xx_A()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    m_pMem->writeByte( addr, A );
}

void Z80_CPU::M_LD_OF_xx_BC()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    m_pMem->writeByte( addr, C );
    addr++;
    m_pMem->writeByte( addr, B );
}

void Z80_CPU::M_LD_OF_xx_DE()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    m_pMem->writeByte( addr, E );
    addr++;
    m_pMem->writeByte( addr, D );
}

void Z80_CPU::M_LD_OF_xx_HL()
{
    WORD addr = m_pMem->readWord(PC);
    PC += 2;
    m_pMem->writeByte( addr, L );
    addr++;
    m_pMem->writeByte( addr, H );
}

void Z80_CPU::M_LD_OF_xx_SP()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    m_pMem->writeByte( addr, (BYTE) SP );
    addr++;
    m_pMem->writeByte( addr, (BYTE) (SP >> 8) );
}

void Z80_CPU::M_LD_BC_OF_xx()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    C = m_pMem->readByte( addr );
    addr++;
    B = m_pMem->readByte( addr );
}

void Z80_CPU::M_LD_DE_OF_xx()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    E = m_pMem->readByte( addr );
    addr++;
    D = m_pMem->readByte( addr );
}

void Z80_CPU::M_LD_HL_OF_xx()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    L = m_pMem->readByte( addr );
    addr++;
    H = m_pMem->readByte( addr );
}

void Z80_CPU::M_LD_SP_OF_xx()
{
    UWORD addr = m_pMem->readWord(PC);
    PC += 2;
    SP = m_pMem->readWord( addr );
}

void Z80_CPU::M_LD_OF_IX_A()
{
    // write A content to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    m_pMem->writeByte( addr, A );
}

void Z80_CPU::M_LD_OF_IX_B()
{
    // write B content to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    m_pMem->writeByte( addr, B );
}

void Z80_CPU::M_LD_OF_IX_C()
{
    // write C content to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    m_pMem->writeByte( addr, C );
}

void Z80_CPU::M_LD_OF_IX_D()
{
    // write D content to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    m_pMem->writeByte( addr, D );
}

void Z80_CPU::M_LD_OF_IX_E()
{
    // write E content to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    m_pMem->writeByte( addr, E );
}

void Z80_CPU::M_LD_OF_IX_H()
{
    // write H content to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    m_pMem->writeByte( addr, H );
}

void Z80_CPU::M_LD_OF_IX_L()
{
    // write L content to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    m_pMem->writeByte( addr, L );
}

void Z80_CPU::M_LD_OF_IX_x()
{
    // write byte to (IX+offset)
    UWORD addr = i_getOffsetAddr(IX);
    BYTE val = m_pMem->readByte(PC);
    PC++;
    m_pMem->writeByte( addr, val );
}

void Z80_CPU::M_LD_OF_IY_A()
{
    // write A content to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    m_pMem->writeByte( addr, A );
}

void Z80_CPU::M_LD_OF_IY_B()
{
    // write B content to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    m_pMem->writeByte( addr, B );
}

void Z80_CPU::M_LD_OF_IY_C()
{
    // write C content to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    m_pMem->writeByte( addr, C );
}

void Z80_CPU::M_LD_OF_IY_D()
{
    // write D content to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    m_pMem->writeByte( addr, D );
}

void Z80_CPU::M_LD_OF_IY_E()
{
    // write E content to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    m_pMem->writeByte( addr, E );
}

void Z80_CPU::M_LD_OF_IY_H()
{
    // write H content to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    m_pMem->writeByte( addr, H );
}

void Z80_CPU::M_LD_OF_IY_L()
{
    // write L content to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    m_pMem->writeByte( addr, L );
}

void Z80_CPU::M_LD_OF_IY_x()
{
    // write byte to (IY+offset)
    UWORD addr = i_getOffsetAddr(IY);
    BYTE val = m_pMem->readByte(PC);
    PC++;
    m_pMem->writeByte( addr, val );
}

void Z80_CPU::M_LD_BC_xx ()
{
    // read 2 immediate arguments
    C = m_pMem->readByte(PC);
    PC++;
    B = m_pMem->readByte(PC);
    PC++;
}

void Z80_CPU::M_LD_DE_xx ()
{
    // read 2 immediate arguments
    E = m_pMem->readByte(PC);
    PC++;
    D = m_pMem->readByte(PC);
    PC++;
}

void Z80_CPU::M_LD_HL_xx ()
{
    // read 2 immediate arguments
    L = m_pMem->readByte(PC);
    PC++;
    H = m_pMem->readByte(PC);
    PC++;
}

void Z80_CPU::M_LD_SP_xx ()
{
    // read 2 immediate arguments
    SP = m_pMem->readWord(PC);
    PC += 2;
}

void Z80_CPU::M_LD_IX_xx ()
{
    // read 2 immediate arguments
    IX = m_pMem->readWord(PC);
    PC += 2;
}

void Z80_CPU::M_LD_IY_xx ()
{
    // read 2 immediate arguments
    IY = m_pMem->readWord(PC);
    PC += 2;
}

void Z80_CPU::M_LD_SP_HL ()
{
    SP = MAKE_WORD( H, L );
}

void Z80_CPU::M_LD_SP_IX ()
{
    SP = IX;
}

void Z80_CPU::M_LD_SP_IY ()
{
    SP = IY;
}

void Z80_CPU::M_LDDR ()
{
    WORD addrSrc = MAKE_WORD(H, L);
    WORD addrDst = MAKE_WORD(D, E);
    WORD counter = MAKE_WORD(B, C);
    BYTE val = m_pMem->readByte( addrSrc );
    m_pMem->writeByte( addrDst, val );
    addrSrc--;
    addrDst--;
    counter--;
    SPLIT_WORD(addrSrc, H, L);
    SPLIT_WORD(addrDst, D, E);
    SPLIT_WORD(counter, B, C);

    // loop
    if( counter != 0)
        PC -= 2;
}

void Z80_CPU::M_LDIR ()
{
    WORD addrSrc = MAKE_WORD(H, L);
    WORD addrDst = MAKE_WORD(D, E);
    WORD counter = MAKE_WORD(B, C);
    BYTE val = m_pMem->readByte( addrSrc );
    m_pMem->writeByte( addrDst, val );
    addrSrc++;
    addrDst++;
    counter--;
    SPLIT_WORD(addrSrc, H, L);
    SPLIT_WORD(addrDst, D, E);
    SPLIT_WORD(counter, B, C);

    // loop
    if( counter != 0)
        PC -= 2;
}

void Z80_CPU::M_NEG()
{
    BYTE res = 0;
    i_sub(res, A);
    A = res;
}

void Z80_CPU::M_POP_AF()
{
    i_pop( A, F );
}

void Z80_CPU::M_POP_BC()
{
    i_pop( B, C );
}

void Z80_CPU::M_POP_DE()
{
    i_pop( D, E );
}

void Z80_CPU::M_POP_HL()
{
    i_pop( H, L );
}

void Z80_CPU::M_POP_IX()
{
    IX = i_popW();
}

void Z80_CPU::M_POP_IY()
{
    IY = i_popW();
}

void Z80_CPU::M_RET()
{
    PC = i_popW();
}

void Z80_CPU::M_RET_C()
{
    if( fl_get_C() )
        PC = i_popW();
}

void Z80_CPU::M_RET_M()
{
    if( fl_get_S() )
        PC = i_popW();
}

void Z80_CPU::M_RET_NC()
{
    if( ! fl_get_C() )
        PC = i_popW();
}

void Z80_CPU::M_RET_NZ()
{
    if( ! fl_get_Z() )
        PC = i_popW();
}

void Z80_CPU::M_RET_P()
{
    // check if positive

    if( ! fl_get_S() )
        PC = i_popW();
}

void Z80_CPU::M_RET_PO()
{
    if( ! fl_get_PV() )
        PC = i_popW();
}

void Z80_CPU::M_RET_PE()
{
    if( fl_get_PV() )
        PC = i_popW();
}

void Z80_CPU::M_RET_Z()
{
    if( fl_get_Z() )
        PC = i_popW();
}



void Z80_CPU::M_PUSH_AF()
{
    i_push( A, F );
}

void Z80_CPU::M_PUSH_BC()
{
    i_push( B, C );
}

void Z80_CPU::M_PUSH_DE()
{
    i_push( D, E );
}

void Z80_CPU::M_PUSH_HL()
{
    i_push( H, L );
}

void Z80_CPU::M_PUSH_IX()
{
    i_pushW( IX );
}

void Z80_CPU::M_PUSH_IY()
{
    i_pushW( IY );
}

void Z80_CPU::M_OR_A()
{
    i_or( A );
}

void Z80_CPU::M_OR_B()
{
    i_or( B );
}

void Z80_CPU::M_OR_C()
{
    i_or( C );
}

void Z80_CPU::M_OR_D()
{
    i_or( D );
}

void Z80_CPU::M_OR_E()
{
    i_or( E );
}

void Z80_CPU::M_OR_H()
{
    i_or( H );
}

void Z80_CPU::M_OR_L()
{
    i_or( L );
}

void Z80_CPU::M_OR_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    i_or( val );
}

void Z80_CPU::M_OR_OF_HL()
{
    // read byte from (HL)
    BYTE val = m_pMem->readByte(MAKE_WORD(H,L));
    i_or( val );
}

void Z80_CPU::M_OUT_OF_x_A()
{
    // read 2 immediate arguments
    BYTE addr = m_pMem->readByte(PC);
    PC++;
    m_pMem->writeByteIO( addr, A  );

}

void Z80_CPU::M_SET_B(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    B = B | mask;
}

void Z80_CPU::M_SET_C(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    C = C | mask;
}

void Z80_CPU::M_SET_D(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    D = D | mask;
}

void Z80_CPU::M_SET_E(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    E = E | mask;
}

void Z80_CPU::M_SET_H(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    H = H | mask;
}

void Z80_CPU::M_SET_L(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    L = L | mask;
}

void Z80_CPU::M_SET_A(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    A = A | mask;
}

void Z80_CPU::M_SET_OF_HL(const int bitIndx)
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));

    BYTE mask = 0x01 << bitIndx;
    bArg2 = bArg2 | mask;
    m_pMem->writeByte(MAKE_WORD(H,L), bArg2);
}

int Z80_CPU::bit_op_IY()
{
    // read byte from (IY+offset)
    BYTE offset = m_pMem->readByte(PC); 
    PC++;
    UWORD addrArg = IY + (signed char) offset;
    BYTE val = m_pMem->readByte(addrArg);
    BYTE byte4 = m_pMem->readByte(PC); 
    PC++;
    const int bitIndx = (byte4 & 0x38) >> 3;
    BYTE opType = byte4 & 0xC0;
    switch (opType)
    {
        case 0x40:  // BIT
            i_bit(val, bitIndx);
            Monitor::step("BIT b, (IY+d)", 4);
            break;
        case 0xC0:  // SET
            i_set(val, bitIndx);
            m_pMem->writeByte(addrArg, val);
            Monitor::step("SET b, (IY+d)", 4);
            break;
        case 0x80:  // RES
            i_res(val, bitIndx);
            m_pMem->writeByte(addrArg, val);
            Monitor::step("RES b, (IY+d)", 4);
            break;
        default:
            Monitor::step("ERR bit_op_IY", 4);
            Monitor::dump();
            return -1;
    }
    return 0;
}

int Z80_CPU::bit_op_IX()
{
    // read byte from (IX+offset)
    BYTE offset = m_pMem->readByte(PC); 
    PC++;
    UWORD addrArg = IX + (signed char) offset;
    BYTE val = m_pMem->readByte(addrArg);
    BYTE byte4 = m_pMem->readByte(PC); 
    PC++;
    const int bitIndx = (byte4 & 0x38) >> 3;
    BYTE opType = byte4 & 0xC0;
    switch (opType)
    {
        case 0x40:  // BIT
            i_bit(val, bitIndx);
            Monitor::step("BIT b, (IX+d)", 4);
            break;
        case 0xC0:  // SET
            i_set(val, bitIndx);
            m_pMem->writeByte(addrArg, val);
            Monitor::step("SET b, (IX+d)", 4);
            break;
        case 0x80:  // RES
            i_res(val, bitIndx);
            m_pMem->writeByte(addrArg, val);
            Monitor::step("RES b, (IX+d)", 4);
            break;
        default:
            Monitor::step("ERR bit_op_IX", 4);
            Monitor::dump();
            return -1;
    }
    return 0;
}

void Z80_CPU::i_bit( const BYTE val, const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    fl_set_Z( (mask & val) == 0x00 );

    // const
    fl_set_H( true );
    fl_set_N( false );
}

void Z80_CPU::i_set(BYTE& val, const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    val = val | mask;
}

void Z80_CPU::i_res(BYTE& val, const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( val & mask )
        val ^= mask;
}

void Z80_CPU::M_SBC_OF_HL()
{
    // read byte from (HL)
    BYTE val = m_pMem->readByte(MAKE_WORD(H,L));
    i_sub( A, val );
}

void Z80_CPU::M_SBC_A()
{
    i_sub_withC( A, A );
}

void Z80_CPU::M_SBC_B()
{
    i_sub_withC( A, B );
}

void Z80_CPU::M_SBC_C()
{
    i_sub_withC( A, C );
}

void Z80_CPU::M_SBC_D()
{
    i_sub_withC( A, D );
}

void Z80_CPU::M_SBC_E()
{
    i_sub_withC( A, E );
}

void Z80_CPU::M_SBC_H()
{
    i_sub_withC( A, H );
}

void Z80_CPU::M_SBC_L()
{
    i_sub_withC( A, L );
}

void Z80_CPU::M_SBC_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    i_sub_withC( A, val );
}

void Z80_CPU::M_SBC_HL_BC()
{
    i_subW_withC( H, L, B, C );
}

void Z80_CPU::M_SBC_HL_DE()
{
    i_subW_withC( H, L, D, E );
}

void Z80_CPU::M_SBC_HL_HL()
{
    i_subW_withC( H, L, H, L );
}

void Z80_CPU::M_SBC_HL_SP()
{
    i_subW_withC( H, L, (BYTE)(SP >> 8), (BYTE) SP );
}

void Z80_CPU::M_SUB_OF_HL()
{
    // read byte from (HL)
    BYTE val = m_pMem->readByte(MAKE_WORD(H,L));
    i_sub( A, val );
}

void Z80_CPU::M_SUB_OF_IX()
{
    // read byte from (IX+d)
    UWORD addr = i_getOffsetAddr( IX );
    BYTE val = m_pMem->readByte(addr);
    i_sub( A, val );
}

void Z80_CPU::M_SUB_OF_IY()
{
    // read byte from (IY+d)
    UWORD addr = i_getOffsetAddr( IY );
    BYTE val = m_pMem->readByte(addr);
    i_sub( A, val );
}

void Z80_CPU::M_SUB_A()
{
    i_sub( A, A );
}

void Z80_CPU::M_SUB_B()
{
    i_sub( A, B );
}

void Z80_CPU::M_SUB_C()
{
    i_sub( A, C );
}

void Z80_CPU::M_SUB_D()
{
    i_sub( A, D );
}

void Z80_CPU::M_SUB_E()
{
    i_sub( A, E );
}

void Z80_CPU::M_SUB_H()
{
    i_sub( A, H );
}

void Z80_CPU::M_SUB_L()
{
    i_sub( A, L );
}

void Z80_CPU::M_SUB_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    i_sub( A, val );
}

void Z80_CPU::M_RES_B(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( B & mask)
        B ^= mask;
}

void Z80_CPU::M_RES_C(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( C & mask)
        C ^= mask;
}

void Z80_CPU::M_RES_D(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( D & mask)
        D ^= mask;}

void Z80_CPU::M_RES_E(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( E & mask)
        E ^= mask;
}

void Z80_CPU::M_RES_H(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( H & mask)
        H ^= mask;
}

void Z80_CPU::M_RES_L(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( L & mask)
        L ^= mask;
}

void Z80_CPU::M_RES_A(const int bitIndx)
{
    BYTE mask = 0x01 << bitIndx;
    if( A & mask)
        A ^= mask;
}

void Z80_CPU::M_RES_OF_HL(const int bitIndx)
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));

    BYTE mask = 0x01 << bitIndx;
    if( bArg2 & mask)
        bArg2 ^= mask;
    m_pMem->writeByte(MAKE_WORD(H,L), bArg2);
}

void Z80_CPU::M_RL_OF_HL()
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));

    i_rl( bArg2 );

    m_pMem->writeByte(MAKE_WORD(H,L), bArg2);
}

void Z80_CPU::M_RL_A()
{
    i_rl( A );
}

void Z80_CPU::M_RL_B()
{
    i_rl( B );
}

void Z80_CPU::M_RL_C()
{
    i_rl( C );
}

void Z80_CPU::M_RL_D()
{
    i_rl( D);
}

void Z80_CPU::M_RL_E()
{
    i_rl( E );
}

void Z80_CPU::M_RL_H()
{
    i_rl( H );
}

void Z80_CPU::M_RL_L()
{
    i_rl( L );
}

void Z80_CPU::M_RLC_OF_HL()
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));

    i_rlc( bArg2 );

    m_pMem->writeByte(MAKE_WORD(H,L), bArg2);
}

void Z80_CPU::M_RLC_A()
{
    i_rlc( A );
}

void Z80_CPU::M_RLC_B()
{
    i_rlc( B );
}

void Z80_CPU::M_RLC_C()
{
    i_rlc( C );
}

void Z80_CPU::M_RLC_D()
{
    i_rlc( D);
}

void Z80_CPU::M_RLC_E()
{
    i_rlc( E );
}

void Z80_CPU::M_RLC_H()
{
    i_rlc( H );
}

void Z80_CPU::M_RLC_L()
{
    i_rlc( L );
}

void Z80_CPU::M_RR_OF_HL()
{
    // read byte from (HL)
    BYTE bArg2 = m_pMem->readByte(MAKE_WORD(H,L));

    i_rr( bArg2 );

    m_pMem->writeByte(MAKE_WORD(H,L), bArg2);
}

void Z80_CPU::M_RR_A()
{
    i_rr( A );
}

void Z80_CPU::M_RR_B()
{
    i_rr( B );
}

void Z80_CPU::M_RR_C()
{
    i_rr( C );
}

void Z80_CPU::M_RR_D()
{
    i_rr( D);
}

void Z80_CPU::M_RR_E()
{
    i_rr( E );
}

void Z80_CPU::M_RR_H()
{
    i_rr( H );
}

void Z80_CPU::M_RR_L()
{
    i_rr( L );
}


void Z80_CPU::i_srl( BYTE& refResult)
{
    // Shift Right Logical 
    bool b0 = ( refResult & 0x01 );
    refResult = refResult >> 1;
    
    refResult &= 0x7F;

    fl_set_C( b0 );
    fl_set_S( false );

    fl_set_H( false );
    fl_set_N( false );
    fl_set_Z (refResult == 0);
    fl_check_parity( refResult );
}

void Z80_CPU::i_rl( BYTE& refResult)
{
    // ROTATE LEFT 
    bool b7 = ( refResult & 0x80 );
    refResult = refResult << 1;
    if( fl_get_C() )
        refResult = refResult | 0x01;

    fl_set_C( b7 );
    fl_set_S( refResult & 0x80 );

    fl_set_H( false );
    fl_set_N( false );
    fl_set_Z (refResult == 0);
    fl_check_parity( refResult );
}

void Z80_CPU::i_rlc( BYTE& refResult)
{
    // ROTATE LEFT 
    bool b7 = ( refResult & 0x80 );
    refResult = refResult << 1;
    if( b7 )
        refResult = refResult | 0x01;

    fl_set_C( b7 );
    fl_set_S( refResult & 0x80 );

    fl_set_H( false );
    fl_set_N( false );
    fl_set_Z (refResult == 0);
    fl_check_parity( refResult );
}

void Z80_CPU::i_rr( BYTE& refResult)
{
    // ROTATE RIGHT 
    bool b0 = ( refResult & 0x01 );
    refResult = refResult >> 1;
    if( fl_get_C() )
        refResult = refResult | 0x80;

    fl_set_C( b0 );
    fl_set_S( refResult & 0x80 );

    fl_set_H( false );
    fl_set_N( false );
    fl_set_Z (refResult == 0);
    fl_check_parity( refResult );
}

void Z80_CPU::M_RLA()
{
    // ROTATE RIGHT ACCUMULATOR
    bool b7 = (A & 0x80);
    bool bWasC= fl_get_C();
    A = A << 1;
    if( bWasC )
        A = A | 0x01;

    fl_set_C( b7 );

    fl_set_H( false );
    fl_set_N( false );

}

void Z80_CPU::M_RRA()
{
    // ROTATE RIGHT ACCUMULATOR
    bool b0 = (A & 0x01);
    bool bWasC= fl_get_C();
    A = A >> 1;
    if( bWasC )
        A = A | 0x80;

    fl_set_C( b0 );

    fl_set_H( false );
    fl_set_N( false );
}

void Z80_CPU::M_RLCA()
{
    // ROTATE LEFT CIRCULAR ACCUMULATOR
    bool b7 = (A & 0x80);
    A = A << 1;
    if( b7 )
        A = A | 0x01;

    fl_set_C( b7 );

    fl_set_H( false );
    fl_set_N( false );
}

void Z80_CPU::M_RRCA()
{
    // ROTATE RIGHT CIRCULAR ACCUMULATOR
    bool b0 = (A & 0x01);
    A = A >> 1;
    if( b0 )
        A = A | 0x80;

    fl_set_C( b0 );

    fl_set_H( false );
    fl_set_N( false );
}

void Z80_CPU::M_RST_00()
{
    i_pushW( PC );
    PC = 0x0000;
}

void Z80_CPU::M_RST_08()
{
    i_pushW( PC );
    PC = 0x0008;
}

void Z80_CPU::M_RST_10()
{
    i_pushW( PC );
    PC = 0x0010;
}

void Z80_CPU::M_RST_18()
{
    i_pushW( PC );
    PC = 0x0018;
}

void Z80_CPU::M_RST_20()
{
    i_pushW( PC );
    PC = 0x0020;
}

void Z80_CPU::M_RST_28()
{
    i_pushW( PC );
    PC = 0x0028;
}

void Z80_CPU::M_RST_30()
{
    i_pushW( PC );
    PC = 0x0030;
}

void Z80_CPU::M_RST_38()
{
    i_pushW( PC );
    PC = 0x0038;
}

void Z80_CPU::M_SCF()
{
    fl_set_C(true);

    // additionally
    fl_set_H( false );
    fl_set_N( false );
}

void Z80_CPU::M_SRL_OF_HL()
{
    // read byte from (HL)
    BYTE val = m_pMem->readByte(MAKE_WORD(H,L));
    i_srl ( val );
    m_pMem->writeByte( MAKE_WORD(H,L), val );
}

void Z80_CPU::M_SRL_A()
{
    i_srl ( A );
}

void Z80_CPU::M_SRL_B()
{
    i_srl ( B );
}

void Z80_CPU::M_SRL_C()
{
    i_srl ( C );
}

void Z80_CPU::M_SRL_D()
{
    i_srl ( D );
}

void Z80_CPU::M_SRL_E()
{
    i_srl ( E );
}

void Z80_CPU::M_SRL_H()
{
    i_srl ( H );
}

void Z80_CPU::M_SRL_L()
{
    i_srl ( L );
}

void Z80_CPU::M_XOR_A()
{
    i_xor( A );
}

void Z80_CPU::M_XOR_B()
{
    i_xor( B );
}

void Z80_CPU::M_XOR_C()
{
    i_xor( C );
}

void Z80_CPU::M_XOR_D()
{
    i_xor( D );
}

void Z80_CPU::M_XOR_E()
{
    i_xor( E );
}

void Z80_CPU::M_XOR_H()
{
    i_xor( H );
}

void Z80_CPU::M_XOR_L()
{
    i_xor( L );
}

void Z80_CPU::M_XOR_x()
{
    // read immediate argument
    BYTE val = m_pMem->readByte(PC);
    PC++;
    i_xor( val );
}

void Z80_CPU::M_XOR_OF_HL()
{
    // read byte from (HL)
    BYTE val = m_pMem->readByte(MAKE_WORD(H,L));
    i_xor( val );
}

// operations internal helpers
void Z80_CPU::i_add( BYTE& refResult, const BYTE op2)
{
    // for flag calculations
    bool bit4A = (refResult & 0x10) != 0;
    bool bit4B = (op2       & 0x10) != 0;

    bool bit7A = (refResult & 0x80) != 0;
    bool bit7B = (op2       & 0x80) != 0;

    WORD tempW = refResult + op2;

    // check the bits
    bool bit4R = (tempW & 0x0010) != 0;
    bool bit7R = (tempW & 0x0080) != 0;
    bool bit8R = (tempW & 0x0100) != 0;

    // check half carry
    if( bit4A == bit4B )
        fl_set_H( bit4R );
    else
        fl_set_H( !bit4R );
    // check overflow
    fl_set_PV( bit7A == bit7B && bit7R != bit7A );
    // set sign flag
    fl_set_S( bit7R );
    // set carry
    fl_set_C( bit8R );
    // set add/sub 
    fl_set_N( false );
        
    // copy the result
    refResult = (BYTE) tempW;

    // set zero flag
    fl_set_Z( refResult == 0 );
}

void Z80_CPU::i_add_withC( BYTE& refResult, const BYTE op2)
{
    // check C flag
    bool bCflag0 = fl_get_C();
    i_add( refResult, op2 );
    if( ! bCflag0 )
        return;
    // recalculate
    bool bPrevCflag = fl_get_C();
    bool bPrevPVflag = fl_get_PV();
    bool bPrevHflag = fl_get_H();

    i_add( refResult, 1 );
    
    // is it correct??
    // use positive value in either 1st or 2nd step
    if( bPrevCflag )
        fl_set_C( true );
    if( bPrevPVflag )
        fl_set_PV( true );
    if( bPrevHflag )
        fl_set_H( true );
}

void Z80_CPU::i_addW( BYTE& refH, BYTE& refL, const BYTE op2H, const BYTE op2L)
{
    // for flag calculations
    bool bit12A = (refH & 0x10) != 0;
    bool bit12B = (op2H & 0x10) != 0;


    int tempDW = (refH << 8) + refL + (op2H << 8) + op2L;

    // check the bits
    bool bit12R = (tempDW & 0x00001000) != 0;
    bool bit16R = (tempDW & 0x00010000) != 0;

    // check half carry
    if( bit12A == bit12B )
        fl_set_H( bit12R );
    else
        fl_set_H( !bit12R );
    // DO NOT check overflow
    // DO NOT set sign flag
    // set carry
    fl_set_C( bit16R );
    // set add/sub 
    fl_set_N( false );
       
    // copy the result
    refH = (BYTE) (tempDW >> 8);
    refL = (BYTE) tempDW;
    // DO NOT set zero flag
}

void Z80_CPU::i_addW( WORD& refw, const BYTE op2H, const BYTE op2L)
{
    // for flag calculations
    bool bit12A = (refw & 0x1000) != 0;
    bool bit12B = (op2H & 0x10) != 0;


    int tempDW = refw + (op2H << 8) + op2L;

    // check the bits
    bool bit12R = (tempDW & 0x00001000) != 0;
    bool bit16R = (tempDW & 0x00010000) != 0;

    // check half carry
    if( bit12A == bit12B )
        fl_set_H( bit12R );
    else
        fl_set_H( !bit12R );
    // DO NOT check overflow
    // DO NOT set sign flag
    // set carry
    fl_set_C( bit16R );
    // set add/sub 
    fl_set_N( false );
       
    // copy the result
    refw = (WORD) tempDW;
    // DO NOT set zero flag
}

void Z80_CPU::i_sub( BYTE& refResult, const BYTE op2)
{
    // for flag calculations
    bool bit4A = (refResult & 0x10) != 0;
    bool bit4B = (op2       & 0x10) != 0;

    bool bit7A = (refResult & 0x80) != 0;
    bool bit7B = (op2       & 0x80) != 0;

    UWORD tempW = refResult - op2;
//std::cout << std::hex << "i_sub " << (int ) refResult << ", " << int (op2) << " = " << tempW << std::endl;

    // check the bits
    bool bit4R = (tempW & 0x0010) != 0;
    bool bit7R = (tempW & 0x0080) != 0;
    bool bit8R = (tempW & 0x0100) != 0;

    // check half carry
    if( bit4A == bit4B )
        fl_set_H( bit4R );
    else
        fl_set_H( !bit4R );
    // check overflow
    fl_set_PV( bit7A != bit7B && bit7R != bit7A );
    // set sign flag
    fl_set_S( bit7R );
    // set carry
    fl_set_C( bit8R );
    // set add/sub 
    fl_set_N( true );
        
    // copy the result
    refResult = (BYTE) tempW;

    // set zero flag
    fl_set_Z( refResult == 0 );
}

void Z80_CPU::i_sub_withC( BYTE& refResult, const BYTE op2)
{
    // check C flag
    bool bCflag0 = fl_get_C();
    i_sub( refResult, op2 );
    if( ! bCflag0 )
        return;
    // recalculate
    bool bPrevCflag = fl_get_C();
    bool bPrevPVflag = fl_get_PV();
    bool bPrevHflag = fl_get_H();

    i_sub( refResult, 1 );
    
    // is it correct??
    // use positive value in either 1st or 2nd step
    if( bPrevCflag )
        fl_set_C( true );
    if( bPrevPVflag )
        fl_set_PV( true );
    if( bPrevHflag )
        fl_set_H( true );
}


void Z80_CPU::i_subW( BYTE& refH, BYTE& refL, const BYTE op2H, const BYTE op2L)
{
    // for flag calculations
    bool bit12A = (refH & 0x10) != 0;
    bool bit12B = (op2H & 0x10) != 0;

    bool bit15A = (refH & 0x80) != 0;
    bool bit15B = (op2H & 0x80) != 0;


    int tempDW = (refH << 8) + refL - (op2H << 8) - op2L;

    // check the bits
    bool bit12R = (tempDW & 0x00001000) != 0;
    bool bit15R = (tempDW & 0x00008000) != 0;
    bool bit16R = (tempDW & 0x00010000) != 0;

    // check sign
    fl_set_S( bit15R );
    // check half carry
    if( bit12A == bit12B )
        fl_set_H( bit12R );
    else
        fl_set_H( !bit12R );
    // check overflow
    fl_set_PV( bit15A != bit15B && bit15R != bit15A );
    // set carry
    fl_set_C( bit16R );
    // set add/sub 
    fl_set_N( true );
       
    // copy the result
    refH = (BYTE) (tempDW >> 8);
    refL = (BYTE) tempDW;

    // set Zero flag
    fl_set_Z( refH==0 && refL==0);
}

void Z80_CPU::i_subW_withC( BYTE& refH, BYTE& refL, const BYTE op2H, const BYTE op2L)
{
    // check C flag
    bool bCflag0 = fl_get_C();
    i_subW( refH, refL, op2H, op2L );
    if( ! bCflag0 )
        return;
    // recalculate
    bool bPrevCflag = fl_get_C();
    bool bPrevPVflag = fl_get_PV();
    bool bPrevHflag = fl_get_H();

    i_subW( refH, refL, 0, 1 );
    
    // is it correct??
    // use positive value in either 1st or 2nd step
    if( bPrevCflag )
        fl_set_C( true );
    if( bPrevPVflag )
        fl_set_PV( true );
    if( bPrevHflag )
        fl_set_H( true );
}

void Z80_CPU::i_and( const BYTE op2 )
{
    A &= op2;

    // set flags

    // set sign flag
    fl_set_S( A & 0x80 );

    // set zero flag
    fl_set_Z( A == 0 );

    // always
    fl_set_H( true );
    fl_set_C( false);
    fl_set_N( false );

    // check parity
    fl_check_parity (A);
}

void Z80_CPU::i_or( const BYTE op2 )
{
    A |= op2;

    // set flags

    // set sign flag
    fl_set_S( A & 0x80 );

    // set zero flag
    fl_set_Z( A == 0 );

    // always
    fl_set_H( false );
    fl_set_C( false);
    fl_set_N( false );

    // check parity
    fl_check_parity (A);
}

void Z80_CPU::i_dec( BYTE& refResult)
{
    fl_set_PV( refResult == 0x80 );

    refResult--;

    fl_set_H( refResult & 0x0F);

    fl_set_S( refResult & 0x80 );
    fl_set_Z( refResult == 0x00 );

    fl_set_N( true );
}

void Z80_CPU::i_inc( BYTE& refResult)
{
    fl_set_PV( refResult == 0x7F );
    fl_set_H( refResult & 0x0F );
    refResult++;

    fl_set_S( refResult & 0x80 );
    fl_set_Z( refResult == 0x00 );

    fl_set_N( false );
}

void Z80_CPU::i_incW( BYTE& refH, BYTE& refL)
{
    WORD dWord = MAKE_WORD(refH, refL);
    dWord++;
    SPLIT_WORD( dWord, refH, refL );
}


void Z80_CPU::i_xor( const BYTE op2 )
{
    A ^= op2;

    // set flags

    // set sign flag
    fl_set_S( A & 0x80 );

    // set zero flag
    fl_set_Z( A == 0 );

    // always
    fl_set_H( false );
    fl_set_C( false);
    fl_set_N( false );

    // check parity
    fl_check_parity (A);
}

void Z80_CPU::i_push( const BYTE bH, const BYTE bL)
{
/*    UWORD a=SP;
    for (int i =0; i<8; i++)
    {
        std::cout << std::hex <<  m_pMem->readWord(a) << std::endl;
        a++; a++;
    }

    std::cout << "HL: " << std::hex << MAKE_WORD(H,L) << std::endl;
*/
    SP--;
    m_pMem->writeByte( SP, bH );
    SP--;
    m_pMem->writeByte( SP, bL );
}

void Z80_CPU::i_pushW( const WORD w)
{
    SP -= 2;
    m_pMem->writeWord( SP, w );
}

void Z80_CPU::i_pop(BYTE& bH, BYTE& bL)
{

    bL = m_pMem->readByte( SP);
    SP++;
    bH = m_pMem->readByte( SP );
    SP++;

}

WORD Z80_CPU::i_popW()
{

    WORD w;
    w = m_pMem->readWord( SP );
    SP += 2;

    return w;
}

UWORD Z80_CPU::i_getOffsetAddr( const UWORD Ireg)
{
    BYTE offset = m_pMem->readByte(PC); 
    PC++;
    UWORD addr = Ireg + (signed char) offset;
    return addr;
}

void Z80_CPU::fl_set_C( bool n)
{
    if( n )
        F = F | FLAGS::FL_C;
    else
        F = F & ~FLAGS::FL_C;
}

void Z80_CPU::fl_set_N( bool n)
{
    if( n )
        F = F | FLAGS::FL_N;
    else
        F = F & ~FLAGS::FL_N;
}

void Z80_CPU::fl_set_PV( bool n)
{
    if( n )
        F = F | FLAGS::FL_PV;
    else
        F = F & ~FLAGS::FL_PV;
}

void Z80_CPU::fl_set_H( bool n)
{
    if( n )
        F = F | FLAGS::FL_H;
    else
        F = F & ~FLAGS::FL_H;
}

void Z80_CPU::fl_set_Z( bool n)
{
    if( n )
        F = F | FLAGS::FL_Z;
    else
        F = F & ~FLAGS::FL_Z;
}

void Z80_CPU::fl_set_S( bool n)
{
    if( n )
        F = F | FLAGS::FL_S;
    else
        F = F & ~FLAGS::FL_S;
}

bool Z80_CPU::fl_get_C()
{
    return F & FLAGS::FL_C;
}

bool Z80_CPU::fl_get_N()
{
    return F & FLAGS::FL_N;
}

bool Z80_CPU::fl_get_PV()
{
    return F & FLAGS::FL_PV;
}

bool Z80_CPU::fl_get_H()
{
    return F & FLAGS::FL_H;
}

bool Z80_CPU::fl_get_Z()
{
    return F & FLAGS::FL_Z;
}

bool Z80_CPU::fl_get_S()
{
    return F & FLAGS::FL_S;
}

void Z80_CPU::fl_check_parity( BYTE op )
{
    bool bParity = true ;
    for( int i = 0; i < 8; i++ )
    {
        if( op & 0x01 )
            bParity = ! bParity;
        op = op >> 1;
    }

    fl_set_PV( bParity );

}

void Z80_CPU::loadRegistersFromImage(BYTE* tab)
{
    int i=0;
    BYTE dumy;
    A = tab[i++];
    F = tab[i++];
    C = tab[i++];
    B = tab[i++];
    L = tab[i++];
    H = tab[i++];
    PC = MAKE_WORD( tab[i+1], tab[i] );
    i += 2;
    SP = MAKE_WORD( tab[i+1], tab[i] );
    i += 2;
    I = tab[i++];
    R = tab[i++];
    dumy = tab[i++]; 
    E = tab[i++]; 
    D = tab[i++]; 
    C_PR = tab[i++];
    B_PR = tab[i++];
    E_PR = tab[i++]; 
    D_PR = tab[i++]; 
    A_PR = tab[i++]; 
    F_PR = tab[i++];
    IY = MAKE_WORD( tab[i+1], tab[i] );
    i += 2;
    IX = MAKE_WORD( tab[i+1], tab[i] );
    i += 2;
    m_bEI = tab[i];
            
    Monitor::dump();
}



