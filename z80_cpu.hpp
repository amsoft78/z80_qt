#ifndef Z80_CPU_HPP
#define Z80_CPU_HPP

#include "mem.hpp"

class Z80_CPU
{
    friend class Monitor;
private:
    // registers
    BYTE A;
    BYTE F;
    
    BYTE B;
    BYTE C;

    BYTE D;
    BYTE E;

    BYTE H;
    BYTE L;

    WORD PC;    // program counter
    WORD SP;    // stack pointer

    WORD IX;
    WORD IY;

    BYTE I; // INTERRUPT vector

    BYTE R; // refresh register

    // shadows - prim registers
    BYTE A_PR;
    BYTE F_PR;
    
    BYTE B_PR;
    BYTE C_PR;

    BYTE D_PR;
    BYTE E_PR;

    BYTE H_PR;
    BYTE L_PR;


    // helpers
    MEM*    m_pMem;

    WORD    M1_PC; // PC state at M1 state (first byte of opcode)

    // flags
    struct FLAGS
    {
        enum
        {
            FL_C    = 0x01,
            FL_N    = 0x02,
            FL_PV   = 0x04,
            FL_H    = 0x10,
            FL_Z    = 0x40,
            FL_S    = 0x80
        } type;
    };

    // methods
    int fetch();
    int handle_prefix_CB();
    int handle_prefix_DD();
    int handle_prefix_ED();
    int handle_prefix_FD();

    // operations - mnemonics
    void M_ADC_A_OF_HL();
    void M_ADC_A_OF_IX();
    void M_ADC_A_OF_IY();

    void M_ADC_A_A();
    void M_ADC_A_B();
    void M_ADC_A_C();
    void M_ADC_A_D();
    void M_ADC_A_E();
    void M_ADC_A_H();
    void M_ADC_A_L();

    void M_ADC_A_x();

    void M_ADD_A_OF_HL();
    void M_ADD_A_OF_IX();
    void M_ADD_A_OF_IY();

    void M_ADD_A_A();
    void M_ADD_A_B();
    void M_ADD_A_C();
    void M_ADD_A_D();
    void M_ADD_A_E();
    void M_ADD_A_H();
    void M_ADD_A_L();

    void M_ADD_A_x();

    void M_ADD_HL_BC();
    void M_ADD_HL_DE();
    void M_ADD_HL_HL();
    void M_ADD_HL_SP();
    void M_ADD_IX_BC();

    void M_AND_OF_HL();
    void M_AND_OF_IX();
    void M_AND_OF_IY();

    void M_AND_A();
    void M_AND_B();
    void M_AND_C();
    void M_AND_D();
    void M_AND_E();
    void M_AND_H();
    void M_AND_L();

    void M_AND_x();

    void M_BIT_B(const int bitIndx);
    void M_BIT_C(const int bitIndx);
    void M_BIT_D(const int bitIndx);
    void M_BIT_E(const int bitIndx);
    void M_BIT_H(const int bitIndx);
    void M_BIT_L(const int bitIndx);
    void M_BIT_A(const int bitIndx);
    void M_BIT_OF_HL(const int bitIndx);

    void M_CALL();
    void M_CALL_C();
    void M_CALL_M();
    void M_CALL_NC();
    void M_CALL_NZ();
    void M_CALL_P();
    void M_CALL_PE();
    void M_CALL_PO();
    void M_CALL_Z();

    void M_CCF();

    void M_CP_OF_HL();
    void M_CP_OF_IX();
    void M_CP_OF_IY();

    void M_CP_A();
    void M_CP_B();
    void M_CP_C();
    void M_CP_D();
    void M_CP_E();
    void M_CP_H();
    void M_CP_L();
    void M_CP_x();

    void M_CPL();

    void M_DEC_A();
    void M_DEC_B();
    void M_DEC_C();
    void M_DEC_D();
    void M_DEC_E();
    void M_DEC_H();
    void M_DEC_L();

    void M_DEC_BC();
    void M_DEC_DE();
    void M_DEC_HL();
    void M_DEC_SP();

    void M_DEC_OF_HL();
    void M_DEC_OF_IX();
    void M_DEC_OF_IY();

    void M_DJNZ();

    void M_DI();
    void M_EI();

    void M_EXX();
    void M_EX_AF_AFP();
    void M_EX_DE_HL();
    void M_EX_OF_SP_HL();
    void M_EX_OF_SP_IX();
    void M_EX_OF_SP_IY();

    void M_HALT();

    void M_IM(int m);

    void M_IN_A_OF_C();
    void M_IN_A_OF_x();

    void M_INC_A();
    void M_INC_B();
    void M_INC_C();
    void M_INC_D();
    void M_INC_E();
    void M_INC_H();
    void M_INC_L();

    void M_INC_BC();
    void M_INC_DE();
    void M_INC_HL();
    void M_INC_SP();
    void M_INC_IX();
    void M_INC_IY();

    void M_INC_OF_HL();
    void M_INC_OF_IX();
    void M_INC_OF_IY();

    void M_JP();
    void M_JP_OF_HL();
    void M_JP_OF_IX();
    void M_JP_OF_IY();

    void M_JP_C();
    void M_JP_M();
    void M_JP_NC();
    void M_JP_NZ();
    void M_JP_P();
    void M_JP_PE();
    void M_JP_PO();
    void M_JP_Z();

    void M_JR_C();
    void M_JR_NC();
    void M_JR_NZ();
    void M_JR_Z();
    void M_JR_x();

    void M_LD_HL_OF_xx();

    void M_LD_OF_xx_A();

    void M_LD_A_OF_BC();
    void M_LD_A_OF_DE();
    void M_LD_A_OF_HL();
    void M_LD_A_OF_IX();
    void M_LD_A_OF_IY();
    
    void M_LD_A_A();
    void M_LD_A_B();
    void M_LD_A_C();
    void M_LD_A_D();
    void M_LD_A_E();
    void M_LD_A_H();
    void M_LD_A_I();
    void M_LD_A_L();
    void M_LD_A_R();
    void M_LD_A_OF_xx();
    void M_LD_A_x();

    void M_LD_B_OF_HL();
    void M_LD_B_OF_IX();
    void M_LD_B_OF_IY();
    void M_LD_B_A ();
    void M_LD_B_C();
    void M_LD_B_D();
    void M_LD_B_E();
    void M_LD_B_H();
    void M_LD_B_L();
    void M_LD_B_x();

    void M_LD_C_OF_HL();
    void M_LD_C_OF_IX();
    void M_LD_C_OF_IY();
    void M_LD_C_A ();
    void M_LD_C_B();
    void M_LD_C_D();
    void M_LD_C_E();
    void M_LD_C_H();
    void M_LD_C_L();
    void M_LD_C_x();

    void M_LD_D_OF_HL();
    void M_LD_D_OF_IX();
    void M_LD_D_OF_IY();
    void M_LD_D_A ();
    void M_LD_D_B();
    void M_LD_D_C();
    void M_LD_D_E();
    void M_LD_D_H();
    void M_LD_D_L();
    void M_LD_D_x();

    void M_LD_E_OF_HL();
    void M_LD_E_OF_IX();
    void M_LD_E_OF_IY();
    void M_LD_E_A ();
    void M_LD_E_B();
    void M_LD_E_C();
    void M_LD_E_D();
    void M_LD_E_H();
    void M_LD_E_L();
    void M_LD_E_x();

    void M_LD_H_OF_HL();
    void M_LD_H_OF_IX();
    void M_LD_H_OF_IY();
    void M_LD_H_A ();
    void M_LD_H_B();
    void M_LD_H_C();
    void M_LD_H_D();
    void M_LD_H_E();
    void M_LD_H_H();
    void M_LD_H_L();
    void M_LD_H_x();

    void M_LD_I_A ();

    void M_LD_L_OF_HL();
    void M_LD_L_OF_IX();
    void M_LD_L_OF_IY();
    void M_LD_L_A ();
    void M_LD_L_B();
    void M_LD_L_C();
    void M_LD_L_D();
    void M_LD_L_E();
    void M_LD_L_H();
    void M_LD_L_L();
    void M_LD_L_x();

    void M_LD_OF_xx_BC();
    void M_LD_OF_xx_DE();
    void M_LD_OF_xx_HL();
    void M_LD_OF_xx_SP();

    void M_LD_OF_BC_A();
    void M_LD_OF_DE_A();
    void M_LD_OF_HL_A();
    void M_LD_OF_HL_B();
    void M_LD_OF_HL_C();
    void M_LD_OF_HL_D();
    void M_LD_OF_HL_E();
    void M_LD_OF_HL_H();
    void M_LD_OF_HL_L();
    void M_LD_OF_HL_x();

    void M_LD_OF_IX_A();
    void M_LD_OF_IX_B();
    void M_LD_OF_IX_C();
    void M_LD_OF_IX_D();
    void M_LD_OF_IX_E();
    void M_LD_OF_IX_H();
    void M_LD_OF_IX_L();
    void M_LD_OF_IX_x();

    void M_LD_OF_IY_A();
    void M_LD_OF_IY_B();
    void M_LD_OF_IY_C();
    void M_LD_OF_IY_D();
    void M_LD_OF_IY_E();
    void M_LD_OF_IY_H();
    void M_LD_OF_IY_L();
    void M_LD_OF_IY_x();

    void M_LD_BC_OF_xx ();
    void M_LD_DE_OF_xx ();
    void M_LD_SP_OF_xx();

    void M_LD_BC_xx ();
    void M_LD_DE_xx ();
    void M_LD_HL_xx ();
    void M_LD_SP_xx ();
    void M_LD_IX_xx ();
    void M_LD_IY_xx ();

    void M_LD_SP_HL();
    void M_LD_SP_IX();
    void M_LD_SP_IY();

    void M_LDDR();
    void M_LDIR();

    void M_NEG();

    void M_OR_A();
    void M_OR_B();
    void M_OR_C();
    void M_OR_D();
    void M_OR_E();
    void M_OR_H();
    void M_OR_L();
    void M_OR_x();
    void M_OR_OF_HL();

    void M_OUT_OF_x_A();

    void M_PUSH_AF();
    void M_PUSH_BC();
    void M_PUSH_DE();
    void M_PUSH_HL();
    void M_PUSH_IX();
    void M_PUSH_IY();

    void M_POP_AF();
    void M_POP_BC();
    void M_POP_DE();
    void M_POP_HL();
    void M_POP_IX();
    void M_POP_IY();

    void M_RET();
    void M_RET_C();
    void M_RET_M();
    void M_RET_NC();
    void M_RET_NZ();
    void M_RET_P();
    void M_RET_PE();
    void M_RET_PO();
    void M_RET_Z();

    void M_RST_00();
    void M_RST_08();
    void M_RST_10();
    void M_RST_18();
    void M_RST_20();
    void M_RST_28();
    void M_RST_30();
    void M_RST_38();

    void M_RES_B(const int bitIndx);
    void M_RES_C(const int bitIndx);
    void M_RES_D(const int bitIndx);
    void M_RES_E(const int bitIndx);
    void M_RES_H(const int bitIndx);
    void M_RES_L(const int bitIndx);
    void M_RES_A(const int bitIndx);
    void M_RES_OF_HL(const int bitIndx);

    void M_RLA();
    void M_RRA();
    void M_RLCA();
    void M_RRCA();

    void M_RL_OF_HL();
    void M_RL_A();
    void M_RL_B();
    void M_RL_C();
    void M_RL_D();
    void M_RL_E();
    void M_RL_H();
    void M_RL_L();

    void M_RLC_OF_HL();
    void M_RLC_A();
    void M_RLC_B();
    void M_RLC_C();
    void M_RLC_D();
    void M_RLC_E();
    void M_RLC_H();
    void M_RLC_L();

    void M_RR_OF_HL();
    void M_RR_A();
    void M_RR_B();
    void M_RR_C();
    void M_RR_D();
    void M_RR_E();
    void M_RR_H();
    void M_RR_L();

    void M_SET_B(const int bitIndx);
    void M_SET_C(const int bitIndx);
    void M_SET_D(const int bitIndx);
    void M_SET_E(const int bitIndx);
    void M_SET_H(const int bitIndx);
    void M_SET_L(const int bitIndx);
    void M_SET_A(const int bitIndx);
    void M_SET_OF_HL(const int bitIndx);

    void M_SBC_OF_HL();
    void M_SBC_A();
    void M_SBC_B();
    void M_SBC_C();
    void M_SBC_D();
    void M_SBC_E();
    void M_SBC_H();
    void M_SBC_L();
    void M_SBC_x();

    void M_SBC_HL_BC();
    void M_SBC_HL_DE();
    void M_SBC_HL_HL();
    void M_SBC_HL_SP();

    void M_SCF();

    void M_SRL_OF_HL();
    void M_SRL_A();
    void M_SRL_B();
    void M_SRL_C();
    void M_SRL_D();
    void M_SRL_E();
    void M_SRL_H();
    void M_SRL_L();

    void M_SUB_OF_HL();
    void M_SUB_OF_IX();
    void M_SUB_OF_IY();
    void M_SUB_A();
    void M_SUB_B();
    void M_SUB_C();
    void M_SUB_D();
    void M_SUB_E();
    void M_SUB_H();
    void M_SUB_L();
    void M_SUB_x();

    void M_XOR_A();
    void M_XOR_B();
    void M_XOR_C();
    void M_XOR_D();
    void M_XOR_E();
    void M_XOR_H();
    void M_XOR_L();
    void M_XOR_x();
    void M_XOR_OF_HL();

    // operations internal helpers
    void i_add( BYTE& refResult, const BYTE op2);
    void i_add_withC( BYTE& refResult, const BYTE op2);
    void i_addW( BYTE& refH, BYTE& refL, const BYTE op2H, const BYTE op2L);
    void i_addW( WORD& refw, const BYTE op2H, const BYTE op2L);

    void i_sub( BYTE& refResult, const BYTE op2);
    void i_sub_withC( BYTE& refResult, const BYTE op2);
    void i_subW( BYTE& refH, BYTE& refL, const BYTE op2H, const BYTE op2L);
    void i_subW_withC( BYTE& refH, BYTE& refL, const BYTE op2H, const BYTE op2L);

    void i_dec( BYTE& refResult);
    void i_inc( BYTE& refResult);
    void i_incW( BYTE& refH, BYTE& refL);

    void i_and( const BYTE op2 );
    void i_or( const BYTE op2 );
    void i_xor( const BYTE op2 );

    void i_push( const BYTE bH, const BYTE bL );
    void i_pushW( const WORD w);
    void i_pop( BYTE& bH, BYTE& bL );
    WORD i_popW();

    void i_rl( BYTE& refResult);
    void i_rlc( BYTE& refResult);
    void i_rr( BYTE& refResult);

    void i_srl( BYTE& refResult);

    UWORD i_getOffsetAddr( const UWORD Ireg);
    
    int bit_op_IY();
    int bit_op_IX();

    void i_bit(const BYTE val, const int bitIndx );
    void i_set( BYTE& val, const int bitIndx );
    void i_res( BYTE& val, const int bitIndx );

    // flag setters
    void fl_set_C( bool n);
    void fl_set_N( bool n);
    void fl_set_PV( bool n);
    void fl_set_H( bool n);
    void fl_set_Z( bool n);
    void fl_set_S( bool n);

    // flag getters
    bool fl_get_C();
    bool fl_get_N();
    bool fl_get_PV();
    bool fl_get_H();
    bool fl_get_Z();
    bool fl_get_S();

    //helper
    void fl_check_parity( BYTE op ); // sets the parity flag according to value

    bool m_bInterrupt;
    bool m_bReset;
    bool m_bEI;
    bool m_bHalt;

public:

    Z80_CPU( MEM* pMem);

    void reset(UWORD nPC = 0x0000);
    void executionLoop();
    void interrupt();
    void setreset();

    void loadRegistersFromImage(BYTE* tab);

};

class Monitor
{
public:
    static Z80_CPU* pCPU;
    static void step( const char* sLabel, const int s );
    static void dump( );
    static void reset( );
    static void clearTables( );
    static void on()
    {
        m_bOn = true;
    }
    static void off()
    {
        m_bOn = false;
    }
private:
    static bool m_bTable8[0x100];
    static bool m_bTable16[0x10000];
    static bool m_bOn;
};


#endif // Z80_CPU_HPP

