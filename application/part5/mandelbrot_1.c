#include <trykernel.h>

/*-------------------------------------------------------------------------
 * マンデルブロ集合の計算
 */
#define WIDTH       80
#define HEIGHT      24
#define MAX_ITER    1000

char output[HEIGHT][WIDTH+1];
const char symbols[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/*tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

void mandelbrot(char output[HEIGHT][WIDTH+1], int h_start, int h_end) {
    int x, y, iteration;
    double real, imag, zr, zi, zr2, zi2;
    int symlen = sizeof(symbols) - 1;
    
    for (y = h_start; y < h_end; y++) {
        for (x = 0; x < WIDTH; x++) {
            real = (x - WIDTH / 2.0) * 3.0 / WIDTH - 0.5;
            imag = (y - HEIGHT / 2.0) * 2.0 / HEIGHT;
            zr = 0.0;
            zi = 0.0;
            iteration = 0;

            while ((zr * zr + zi * zi <= 4.0) && (iteration < MAX_ITER)) {
                // Preserve values before updating
                zr2 = zr * zr - zi * zi + real;
                zi2 = 2.0 * zr * zi + imag;

                zr = zr2;
                zi = zi2;
                
                iteration++;
            }
            int index = iteration == MAX_ITER ? 0 : iteration % symlen;
            output[y][x] = symbols[index];
        }
        output[y][WIDTH] = '\0'; // Null terminate each line
    }
}

/*-------------------------------------------------------------------------
 * イベントフラグ生成情報
 */
ID  flgid;
T_CFLG cflg = {
    .flgatr     = TA_TFIFO | TA_WMUL,
    .iflgptn    = 0,
};

/*-------------------------------------------------------------------------
 * マンデルブロ集合計算タスク
 */
ID  tskid_man;                          // タスクID番号
#define STKSZ_MAN   1024                // スタックサイズ
UW  tskstk_man[STKSZ_MAN/sizeof(UW)];   // スタック領域
void tsk_man(INT stacd, void *exinf);   // タスク実行関数

T_CTSK ctsk_man = {                     // タスク生成情報
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_man,
    .stksz      = STKSZ_MAN,
    .itskpri    = 5,
    .bufptr     = tskstk_man,
};

void tsk_man(INT stacd, void *exinf)
{
    mandelbrot(output, 0, HEIGHT);      // マンデルブロ集合の計算

    tk_set_flg(flgid, 1<<0);            // 完了フラグのセット
    tk_slp_tsk(TMO_FEVR);
}

int usermain(void)
{
    UINT    flgptn;

    tskid_man = tk_cre_tsk(&ctsk_man);  // 計算タスクの生成
    flgid = tk_cre_flg(&cflg);          // イベントフラグの生成

    tm_putstring("Start\n");
    tk_sta_tsk(tskid_man, 0);           // 計算タスクの実行

    tk_wai_flg(flgid, 1<<0, TWF_ANDW, &flgptn, TMO_FEVR);   // 完了フラグ待ち
    tm_putstring("End\n");

    for (INT i = 0; i < HEIGHT; i++) {
        tm_putstring(output[i]); tm_putstring("\n");    // マンデルブロ集合の出力
    }

    tk_slp_tsk(TMO_FEVR);
    return 0;
}