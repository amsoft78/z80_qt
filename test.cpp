//#include <QApplication>
//#include <QLabel>

#include "mainwindow.hpp"
#include "mem.hpp"
#include <iostream>
#include "z80_cpu.hpp"

MainWindow::MainWindow()
{
    QTextCodec::setCodecForTr (QTextCodec::codecForName ("UTF-8"));

/*    button = new QPushButton (tr("&Wciśnij mnie ;)"), this);
    button->setGeometry(25, 15, 150, 75);

    connect(button, SIGNAL(clicked()), qApp, SLOT(quit()));
*/

    setMinimumSize(768, 600);
    resize(768, 600);

}
class ULA_Mem:
public ULA_Mem0
{
public:
    ULA_Mem()
    {
        for( int i = 0; i <0x10000; i++ )
            mIO_in[i]=0xFF;

        resetModif();
    }

    virtual void writeByte (const UWORD address, const BYTE val )
    {
        // recalculate to X and Y
        UWORD addr1 = (UWORD)address - 0x4000;
        unsigned int triada = addr1 >> 11;
        unsigned int lineInTR = addr1 & 0x7FF;
        unsigned int lLine = (lineInTR >> 8) ;
        unsigned int hLine = (lineInTR & 0xE0) >> 5;
//        unsigned int y = hLine * 8 + lLine + triada * 64;
        unsigned int y = (triada << 6) + (hLine << 3) + lLine;
        unsigned int x0 = (addr1 & 0x1F) << 3;
        for( int dx = 0; dx < 8 ; dx ++ )
        {
            // get value
            int tmp = val >> (7-dx);
            if (tmp & 0x01)
                scr [(y<< 8) + x0+dx] = 1;
            else
                scr [(y<< 8) + x0+dx] = 0;
        }

        // mark modified area
        if( m_x1 == -1 || (int)x0 < m_x1 )
            m_x1 = x0;
        if( m_x2 < (int)(x0+7) )
            m_x2 = x0+7;
        if( m_y1 == -1 || (int)y < m_y1 )
            m_y1 = y;
        if( m_y2 < (int)y )
            m_y2 = y;
        
    }

    virtual void writeAttrib (const UWORD address, const BYTE val )
    { 
        // #5800-#5AFF - attribs
        UWORD addr1 = (UWORD)address - 0x5800;
        attr[addr1]=val;
    }

    virtual BYTE readByteIO( const UWORD address ) 
    {
        // kempston
        UWORD addrL = address & 0x00FF;
        if( addrL == 0x1F )
        {
            BYTE res = mIO_in[addrL];
            if( (int)res != 0xFF )
                std::cerr << std::hex << "KEMPSTON: " << (int)res << std::endl;
            return res;
        }

        // keyboard
        BYTE res = mIO_in[address];
        if( res != 0xFF )
            std::cerr << std::hex << "IO: " << (int)res << std::endl;
        // allow more than one read only for shifts
        if( address == 0x7FFE )
        {
            mIO_in[address] |= 0xFD;
        }
        else
            if( address == 0xFEFE )
            {
                mIO_in[address] |= 0xFE;
            }
            else
                mIO_in[address]=0xFF;
        return res;
    }

    void resetModif()
    {
        m_x1 = -1;
        m_y1 = -1;
        m_x2 = -1;
        m_y2 = -1;
    }

    int scr  [256*192];
    int attr [32*24];

    BYTE mIO_in[0x10000];   // just 64KB

    // modified rect
    int m_x1;
    int m_y1;
    int m_x2;
    int m_y2;


};

ULA_Mem ulaMem;

Widget::Widget(QWidget *parent)
:QWidget(parent)
{
    BYTE half = 0xBB;
    // init spectrum palette
    m_colors[0].setRgb(0,0,0);
    m_colors[1].setRgb(0,0,half);
    m_colors[2].setRgb(half,0,0);
    m_colors[3].setRgb(half,0,half);
    m_colors[4].setRgb(0,half,0);
    m_colors[5].setRgb(0,half,half);
    m_colors[6].setRgb(half,half,0);
    m_colors[7].setRgb(half,half,half);

    m_colors[ 8].setRgb(0,0,0);
    m_colors[ 9].setRgb(0,0,0xFF);
    m_colors[10].setRgb(0xFF,0,0);
    m_colors[11].setRgb(0xFF,0,0xFF);
    m_colors[12].setRgb(0,0xFF,0);
    m_colors[13].setRgb(0,0xFF,0xFF);
    m_colors[14].setRgb(0xFF,0xFF,0);
    m_colors[15].setRgb(0xFF,0xFF,0xFF);
    
    m_bInverse = false;
}

void Widget::keyPressEvent( QKeyEvent *k )
{
    if( k->isAutoRepeat () )
    {   k->ignore();
        return;
    }
    std::cerr  <<  std::hex << (int)k->key() << ":" << (int)k->type() << std::endl;
    switch (k->key())
    {
        case 0x1000031: // F2
            Monitor::off();
            break;
        case 0x1000033: // F4
            Monitor::reset();
            break;
        case 0x1000020: // caps shift == shift
            ulaMem.mIO_in[0xFEFE] &= 0xFE;
            break;
        case 0x1000021: // symbol shift == ctrl
            ulaMem.mIO_in[0x7FFE] &= 0xFD;
            break;
        case 0x1000004:   // enter
                ulaMem.mIO_in[0xBFFE] &= 0xFE;
            break;
        case 0x20: // space
                ulaMem.mIO_in[0x7FFE] &= 0xFE;
            break;
        case 0x30: // 0
        case 0x29: // )
                ulaMem.mIO_in[0xEFFE] &= 0xFE;
            break;
        case 0x31: // 1
        case 0x21: // !
                ulaMem.mIO_in[0xF7FE] &= 0xFE;
            break;
        case 0x32: // 2
        case 0x40:
                ulaMem.mIO_in[0xF7FE] &= 0xFD;
            break;
        case 0x33: // 3
                ulaMem.mIO_in[0xF7FE] &= 0xFB;
            break;
        case 0x34: // 4
        case 0x24: // $
                ulaMem.mIO_in[0xF7FE] &= 0xF7;
            break;
        case 0x35: // 5
        case 0x25: // %
                ulaMem.mIO_in[0xF7FE] &= 0xEF;
            break;
        case 0x36: // 6
        case 0x5E: // ^
                ulaMem.mIO_in[0xEFFE] &= 0xEF;
            break;
        case 0x37: // 7
        case 0x26: // &
                ulaMem.mIO_in[0xEFFE] &= 0xF7;
            break;
        case 0x38: // 8
        case 0x2A: // *
                ulaMem.mIO_in[0xEFFE] &= 0xFB;
            break;
        case 0x39: // 9
        case 0x28: // (
                ulaMem.mIO_in[0xEFFE] &= 0xFD;
            break;
        case 0x41: // A
                ulaMem.mIO_in[0xFDFE] &= 0xFE;
            break;
        case 0x42: // B
                ulaMem.mIO_in[0x7FFE] &= 0xEF;
            break;
        case 0x43: // C
                ulaMem.mIO_in[0xFEFE] &= 0xF7;
            break;
        case 0x44: // D
                ulaMem.mIO_in[0xFDFE] &= 0xFB;
            break;
        case 0x45: // E
                ulaMem.mIO_in[0xFBFE] &= 0xFB;
            break;
        case 0x46: // F
                ulaMem.mIO_in[0xFDFE] &= 0xF7;
            break;
        case 0x47: // G
                ulaMem.mIO_in[0xFDFE] &= 0xEF;
            break;
        case 0x48: // H
                ulaMem.mIO_in[0xBFFE] &= 0xEF;
            break;
/*        case 0x: // 
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFE] |= 0x;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFE] &= 0x;
            break;*/
        case 0x4B: // K
                ulaMem.mIO_in[0xBFFE] &= 0xFB;
            break;
        case 0x4C: // L
                ulaMem.mIO_in[0xBFFE] &= 0xFD;
            break;
        case 0x4D: // M
                ulaMem.mIO_in[0x7FFE] &= 0xFB;
            break;
        case 0x50: // P
                ulaMem.mIO_in[0xDFFE] &= 0xFE;
            break;
        case 0x52: // R
                ulaMem.mIO_in[0xFBFE] &= 0xF7;
            break;
        // cursor mapped into kempston
        // Jak widać, bity (od najmłodszego do najstarszego) oznaczają: prawo, lewo, dół, góra, fire
        case 0x1000012: // <-
            ulaMem.mIO_in[0x1F] &= 0xFD; // 2nd
            break;
        case 0x1000013: // |^
            ulaMem.mIO_in[0x1F] &= 0xF7;
            break;
        case 0x1000014: // ->
            ulaMem.mIO_in[0x1F] &= 0xFE;
            break;
        case 0x1000015: // |_
            ulaMem.mIO_in[0x1F] &= 0xFB; // 3rd
            break;
        case 0x1001103: // R_ALT
            ulaMem.mIO_in[0x1F] &= 0xEF; // 5th
            break;
        default:
            k->ignore();
            return;
    }
    k->accept();
}

void Widget::keyReleaseEvent( QKeyEvent *k )
{
    std::cerr  <<  std::hex << (int)k->key() << ":" << (int)k->type() << std::endl;
    switch (k->key())
    {
        case 0x1000020: // caps shift == shift
            ulaMem.mIO_in[0xFEFE] |= 0x01;
            break;
        case 0x1000021: // symbol shift == ctrl
            ulaMem.mIO_in[0x7FFE] |= 0x02;
            break;
        case 0x1000012: // <-
            ulaMem.mIO_in[0x1F] |= 0x02; // 2nd
            break;
        case 0x1000013: // |^
            ulaMem.mIO_in[0x1F] |= 0x08; // 4th
            break;
        case 0x1000014: // ->
            ulaMem.mIO_in[0x1F] |= 0x01;  // 1st
            break;
        case 0x1000015: // |_
            ulaMem.mIO_in[0x1F] |= 0x04; // 3rd
            break;
        case 0x1001103: // R_ALT
            ulaMem.mIO_in[0x1F] |= 0x10; // 5th
            break;

/*        case 0x1000004:   // enter
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xBFFE] |= 0x01;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xBFFE] &= 0xFE;
            break;
        case 0x20: // space
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0x7FFE] |= 0x01;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0x7FFE] &= 0xFE;
            break;
        case 0x31: // 1
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xF7FE] |= 0x01;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xF7FE] &= 0xFE;
            break;
        case 0x32: // 2
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xF7FE] |= 0x02;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xF7FE] &= 0xFD;
            break;
        case 0x33: // 3
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xF7FE] |= 0x04;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xF7FE] &= 0xFB;
            break;
        case 0x41: // A
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFDFE] |= 0x01;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFDFE] &= 0xFE;
            break;
        case 0x42: // B
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0x7FFE] |= 0x10;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0x7FFE] &= 0xEF;
            break;
        case 0x43: // C
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFEFE] |= 0x08;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFEFE] &= 0xF7;
            break;
        case 0x44: // D
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFDFE] |= 0x04;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFDFE] &= 0xFB;
            break;
        case 0x45: // E
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFBFE] |= 0x04;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFBFE] &= 0xFB;
            break;
        case 0x46: // F
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFDFE] |= 0x08;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFDFE] &= 0xF7;
            break;
        case 0x47: // G
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFDFE] |= 0x10;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFDFE] &= 0xEF;
            break;
        case 0x48: // H
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xBFFE] |= 0x10;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xBFFE] &= 0xEF;
            break;
        case 0x4B: // K
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xBFFE] |= 0x04;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xBFFE] &= 0xFB;
            break;
        case 0x4C: // L
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xBFFE] |= 0x02;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xBFFE] &= 0xFD;
            break;
        case 0x4D: // M
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0x7FFE] |= 0x04;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0x7FFE] &= 0xFB;
            break;
        case 0x52: // R
            if( k->type() == QEvent::KeyRelease )
                ulaMem.mIO_in[0xFBFE] |= 0x08;
            if( k->type() == QEvent::KeyPress )
                ulaMem.mIO_in[0xFBFE] &= 0xF7;
            break;

        default:
            k->ignore();
            return;
*/
    }
    k->accept();
//    msleep (40);
}

Widget::~Widget()
{
}

void Widget::paintEvent(QPaintEvent * ev)
{
    QPainter painter(this);
    int x1;
    int y1;
    int x2;
    int y2;
    ev->rect().getCoords(&x1, &y1, &x2, &y2);
    if( x1 < 0 )
        x1 = 0;
    else
        if (x1 > 256*3)  
            x1 = 256;
        else
            x1 = x1 / 3;
    if( x2 < 0 )
        x2 = 0;
    else
        if (x2 > 256*3)  
            x2 = 256;
        else
            x2 = x2 / 3;

    if( y1 < 0 )
        y1 = 0;
    else
        if (y1 > 192*3)  
            y1 = 192;
        else
            y1 = y1 / 3;
    if( y2 < 0 )
        y2 = 0;
    else
        if (y2 > 192*3)  
            y2 = 192;
        else
            y2 = y2 / 3 + 1;

//    for( int y=0; y< 192; y ++ )
    for( int y=y1; y< y2; y ++ )
    {
        BYTE currAttrib = 0;

//        for( int x=0; x<256; x++ )
        for( int x=x1; x< x2; x++ )
        {
            int addr = (y << 8) + x;
            int val = ulaMem.scr[addr];
            // update currAttrib each 8 pixels
            if( (x & 0x07) == 0 )
            {
                int addrAttrib = ((y >> 3) << 5) + ( x >> 3 );
                currAttrib = ulaMem.attr[addrAttrib];
            }
            int colIndx;
            bool bInverse =  ((currAttrib & 0x80) != 0) && m_bInverse;
            if( bInverse )
                val = ! val ;
            if( val )
            {
                colIndx = currAttrib & 0x07;
                if( currAttrib & 0x40 )
                    colIndx += 0x08;
            }
            else
            {
                colIndx = ( currAttrib >> 3 ) & 0x07;
                if( currAttrib & 0x40 )
                    colIndx += 0x08;
            }

            painter.setPen(m_colors[colIndx]);

            painter.drawPoint (3*x,3*y);
            painter.drawPoint (3*x+1,3*y);
            painter.drawPoint (3*x+2,3*y);
            painter.drawPoint (3*x,3*y+1);
            painter.drawPoint (3*x+1,3*y+1);
            painter.drawPoint (3*x+2,3*y+1);
            painter.drawPoint (3*x,3*y+2);
            painter.drawPoint (3*x+1,3*y+2);
            painter.drawPoint (3*x+2,3*y+2);
        }
    }


}

void Widget::updateIfNeeded()
{
    if( ulaMem.m_x1 != -1 )
    {
        this->update( 
            ulaMem.m_x1*3, ulaMem.m_y1*3,
            (ulaMem.m_x2-ulaMem.m_x1+1)*3, (ulaMem.m_y2 - ulaMem.m_y1+1)*3);
    }
}


void ProcThread::run()
{
        FILE *f;
        f = fopen ("rom.rom","rb");
        int i = fread ( m_pMem->mM, 1, 0x4000, f);
        fclose (f);
        std:: cout << "Read: " << i << std::endl;
        Monitor::pCPU = m_pCPU;
        
    if(1)
    {
        FILE *f;
        f = fopen ("z80.z80","rb");

        BYTE prolog[30];

        fread ( prolog, 30, 1, f);

        m_pCPU->loadRegistersFromImage(prolog);

        UWORD addr = 0x4000;
        int ree=0;
        while (! feof(f))
        {
            BYTE x;
            fread ( &x, 1, 1, f);
            ree++;
            if ( x != 0xED )
            {
                m_pMem->writeByte(addr,x);
                addr = addr + (UWORD) 1;
            }
            else
            {
                fread ( &x, 1, 1, f);
                ree++;
                if ( x != 0xED )
                {
                    m_pMem->writeByte(addr,x);
                    addr = addr + (UWORD) 1;
                }
                else
                {
                    // decompress value
                    fread ( &x, 1, 1, f);
                    ree++;
                    BYTE size = x;
                    fread ( &x, 1, 1, f);
                    ree++;
                    for( int i = 0; i< size ; i++ )
                    {
                        m_pMem->writeByte(addr,x);
                        addr = addr + (UWORD) 1;
                    }
                }
            }
            
        }
        fclose (f);
        std::cerr << " z80 read " <<  ree << std::endl;
        std::cerr << std::hex << " Reached addr: " <<  addr << std::endl;
    
    }
    else
        m_pCPU->reset();

    m_pCPU->executionLoop();
}

void TickerThread::run()
{
    int iBlinkCounter = 0;
    while(m_pW)
    {
        msleep(20);
        iBlinkCounter++;
        if( iBlinkCounter == 10 )
        {
            iBlinkCounter = 0;
           // m_pW->blink();
        }
        else
            if( iBlinkCounter & 0x01 ) // 25 FPS
                m_pW->updateIfNeeded();
        m_pCPU->interrupt();
    }
}

void writeWord(const UWORD address,  const WORD val)
{
    bool bADN = address && 0x8000;
    std::cerr << std::dec << "writeWord " << bADN << ": " << address << std::endl;
    std::cerr << std::hex << "writeWord " << bADN << ": " << address << std::endl;
    bool bVLN = val && 0x80;
    std::cerr << std::dec<< "writeWord va; " << bVLN << ": " << val << std::endl;
    std::cerr << std::hex<< "writeWord va; " << bVLN << ": " << val << std::endl;
}

WORD readWord( const UWORD address )
{
    bool bADN = address && 0x80;
    std::cerr << std::dec<< "readWord " << bADN << ": " << address << std::endl;
    std::cerr << std::hex<< "readWord " << bADN << ": " << address << std::endl;
    return 0xcd;
}

int main(int argc, char *argv[])
{

     MEM mem;
     mem.pULA = &ulaMem;
     Z80_CPU cpu(&mem);
    
     Monitor::clearTables();

//     QCoreApplication app(argc, argv);
     QApplication app(argc, argv);
     Widget* pWg = new Widget (0);

     ProcThread procThread;
     procThread.cpuinit( &mem, &cpu );
     procThread.start();

     TickerThread ticker;
     ticker.cpuinit( &cpu );
     ticker.m_pW = pWg;
     ticker.start();
    
 
/*      pWg->connect ( pWg, SIGNAL (changeRequest (State)),
         SLOT (newState (State)) );*/

     pWg->show();
    
     app.exec();
     ticker.m_pW = NULL;
     ticker.wait();
     procThread.wait();  // do not exit before the thread is completed!
  
}


