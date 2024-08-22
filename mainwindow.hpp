#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

//#include <QMainWindow>
//#include <QTextCodec>
//#include <QPushButton>
#include <QtGui>

#include <qwidget.h>

class MainWindow : public QMainWindow
{
public:
   MainWindow();
};

class Widget : public QWidget
{
   Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    void blink()
    {
        m_bInverse = ! m_bInverse; 
        update();
    }

    void updateIfNeeded();

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent( QKeyEvent *k );
    void keyReleaseEvent( QKeyEvent *k );

private:
   QColor m_colors[16];
   bool m_bInverse;     
};

class Z80_CPU;
class MEM;

class ProcThread : public QThread
{
     Q_OBJECT
 private:
     void run();
     Z80_CPU* m_pCPU;
     MEM* m_pMem;

 public:
     void cpuinit(MEM* pMem, Z80_CPU* pCPU)
     {
        m_pCPU = pCPU;
        m_pMem = pMem;
     }
};

class TickerThread : public QThread
{
     Q_OBJECT
 private:
     void run();
     Z80_CPU* m_pCPU;

 public:
     Widget* m_pW;

     void cpuinit(Z80_CPU* pCPU)
     {
        m_pCPU = pCPU;
     }
};
#endif // MAINWINDOW_HPP

