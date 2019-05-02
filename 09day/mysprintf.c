#include <stdarg.h>

//10進数からASCIIコードに変換
static int dec2asc(
    char *str,
    int right,
    int plus,
    int zero,
    int decit,
    int dec)
{
    int minus = 0;
    int len = 0;
    int len_buf = 0; //桁数
    int buf[256] = {};
    if (dec < 0)
    {
        dec *= -1;
        minus = 1;
    }
    while (1)
    {
        //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
        buf[len++] = dec % 10;
        if (dec < 10) break;
        dec /= 10;
    }
    if (minus == 1)
    {
        buf[len++] = '-' - '0';
    }
    else if (plus == 1)
    {
        buf[len++] = '+' - '0';
    }

    if (decit <= len)
    {
        len_buf = len;
    }
    else if (right == 0)
    {
        int diff = decit - len;
        while (diff > 0)
        {
            *(str++) = zero == 0?' ':'0';
            diff--;
        }
        len_buf = decit;
    }

    while (len)
    {
        *(str++) = buf[--len] + '0';
    }
    if (decit > len && right == 1)
    {
        int diff = decit - len;
        while (diff > 0)
        {
            *(str++) = ' ';
            diff--;
        }
    }
    return len_buf;
}

//16進数からASCIIコードに変換
static int hex2asc(
    char *str,
    int right,
    int zero,
    int decit,
    int dec)
{
    //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
    int len = 0;
    int len_buf = 0; //桁数
    int buf[256] = {};
    while (1)
    {
        buf[len++] = dec % 16;
        if (dec < 16) break;
        dec /= 16;
    }

    if (decit <= len)
    {
        len_buf = len;
    }
    else if (right == 0)
    {
        int diff = decit - len;
        while (diff > 0)
        {
            *(str++) = zero == 0?' ':'0';
            diff--;
        }
        len_buf = decit;
    }

    while (len)
    {
        len --;
        *(str++) = (buf[len] < 10)?(buf[len] + '0'):(buf[len] - 10 + 'a');
    }

    if (decit > len && right == 1)
    {
        int diff = decit - len;
        while (diff > 0)
        {
            *(str++) = ' ';
            diff--;
        }
    }
    return len_buf;
}

void mysprintf (char *str, char *fmt, ...)
{
    va_list list;
    int len;
    int right = 0;
    int plus = 0;
    int zero = 0;
    int done = 0;
    int decit = 0;
    va_start (list, fmt);

    while (*fmt)
    {
        if(*fmt=='%')
        {
            fmt++;

            right = 0;
            plus = 0;
            zero = 0;
            done = 0;
            while (done == 0)
            {
                /* flags */
                switch(*fmt)
                {
                    case '-':
                        right = 1;
                        fmt++;
                        break;
                    case '+':
                        plus = 1;
                        fmt++;
                        break;
                    case '0':
                        zero = 1;
                        fmt++;
                        break;
                    default:
                        done = 1;
                        break;
                }
            }

            decit = 0;
            done = 0;
            while (done == 0)
            {
                if ('0' <= *fmt && *fmt <= '9')
                {
                    decit *= 10;
                    decit += *fmt - '0';
                    fmt++;
                }
                else
                {
                    done = 1;
                }

            }

            switch(*fmt)
            {
                case 'd':
                    len = dec2asc(str, right, plus, zero, decit, va_arg (list, int));
                    break;
                case 'x':
                    len = hex2asc(str, right, zero, decit, va_arg (list, int));
                    break;
            }
            str += len; fmt++;
        }
        else
        {
            *(str++) = *(fmt++);
        }
    }
    *str = 0x00; //最後にNULLを追加
    va_end (list);
}
