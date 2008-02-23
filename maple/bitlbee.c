/*-------------------------------------------------------*/
/* bitlbee.c   ( NTHU CS MapleBBS Ver 3.10 )             */
/*-------------------------------------------------------*/
/* author : ono.bbs@lexel.twbbs.org                      */
/* modify : smiler.bbs@bbs.cs.nthu.edu.tw                */
/* target : MSN on MapleBBS                              */
/* create : 05/06/08                                     */
/* update : 08/02/18                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

extern XZ xz[];

static int sock = 0;
static FILE *fw, *fr;
static char buf[512];
static XO bit_xo;

/* 預設上線最多 250 位 */
static BITUSR bit_pool[250];

static
bit_fgets ()
{
  char *tmp;

  fgets (buf, sizeof (buf), fr);
  tmp = strstr (buf, "Logged out");

  if (tmp)
    {
      bit_abort ();
      vmsg ("您在其他地方登入 msn！");
    }
}

static int
bit_body (xo)
     XO *xo;
{
  BITUSR *pp;
  int cnt, n = 2, max;

  cnt = xo->top;
  max = xo->max;
  pp = &bit_pool[cnt];

  while (++n < b_lines)
    {
      move (n, 0);
      if (cnt++ < max)
	{
	  (strstr (pp->status, "Online")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;36m線上\033[m ",
	    cnt, pp->nick, pp->addr) : (strstr (pp->status,
	      "Away")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;33m離開\033[m ",
	    cnt, pp->nick, pp->addr) : (strstr (pp->status,
	      "Busy")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;31m忙碌\033[m ",
	    cnt, pp->nick, pp->addr) : (strstr (pp->status,
	      "Idle")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;34m閒置\033[m ",
	    cnt, pp->nick, pp->addr) : (strstr (pp->status,
	      "Right")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;35m馬上回來\033[m ",
	    cnt, pp->nick, pp->addr) : (strstr (pp->status,
	      "Phone")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m \033[1;32m電話中\033[m ",
	    cnt, pp->nick, pp->addr) : (strstr (pp->status,
	      "Lunch")) ?
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m 外出吃飯 ",
	    cnt, pp->nick,
	    pp->
	    addr) :
	    prints
	    ("%5d   \033[1;37m%-18.17s\033[m  \033[30;1m%-34.33s\033[m %-15.14s ",
	    cnt, pp->nick, pp->addr, pp->status);
	}
      else
	clrtoeol ();

      *pp++;
    };
  return XO_NONE;
}

static int
bit_set (xo)
     XO *xo;
{
  char *tmp, max = 0;
  char seps[] = " \t";

  fprintf (fw, "PRIVMSG root :blist\r\n");
  fflush (fw);

  /* NICK 剛好在第 48 個字, 前面的訊息略過 */
  do
    {
      bit_fgets ();
      if (sock<=0)
	return XO_QUIT;
      tmp = strstr (buf, "Nick");
    }
  while (!tmp);

  for (;;)
    {
      bit_fgets ();
      if (sock <=0)
	return XO_QUIT;

      tmp = strstr (buf, "MSN");

      if (!tmp)
	break;
      else
	{
	  tmp = strtok (buf, ":");
	  tmp = strtok (NULL, seps);
	  sprintf (bit_pool[max].nick, "%s", tmp);
	  tmp = strtok (NULL, seps);
	  sprintf (bit_pool[max].addr, "%s", tmp);
	  tmp = strtok (NULL, seps);
	  tmp = strtok (NULL, "\n");

	  do
	    {
	      *tmp++;
	    }
	  while (*tmp == ' ');

	  sprintf (bit_pool[max].status, "%s", tmp);
	  max++;
	}

    };

  xo->max = max;
  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  return bit_head (xo);
}

/* static */ int
bit_head (xo)
     XO *xo;
{
  clear ();
  vs_head ("MSN 列表", str_site);
  move (1, 0);
  prints
	(" [w]傳訊 [c]改暱稱 [^k]斷線 [a]增刪聯絡人 [d]刪除聯絡人 [l]msn紀錄 [h]說明   \n"
    "\033[30;47m 編號   代   號             信          箱                     狀  態         \033[m");

  return bit_body (xo);
}

static int
bit_init (xo)
     XO *xo;
{
  return bit_set (xo);

}

static int
bit_load (xo)
     XO *xo;
{
  return bit_head (xo);
}


static int
bit_help (xo)
  XO *xo;
{
  xo_help("msn");
  return XO_HEAD;
}

static int
bit_write (xo)
     XO *xo;
{
  int pos;
  char hint[30], *nick, str[65], file[128];
  screenline sl[b_lines + 1];
  FILE *fp;

  pos = xo->pos;
  nick = bit_pool[pos].nick;
  sprintf (hint, "★[%s] ", nick);

  vs_save (sl);

  if (vget (b_lines - 1, 0, hint, str, 60, DOECHO) &&
    vans ("確定要丟出 msn (Y/N)？[Y] ") != 'n')
    {
      usr_fpath (file, cuser.userid, FN_MSN);
      fp = fopen (file, "a");
      fprintf (fp, "To %s (@msn)：%s\n", nick, str);
      fclose (fp);

      fprintf (fw, "PRIVMSG %s :%s\r\n", nick, str);
      fflush (fw);
    }

  vs_restore (sl);
  return XO_INIT;
}

static int
bit_allow (xo)
     XO *xo;
{

  if (vans ("確定要解除封鎖他 (Y/N)？[N]") == 'y')
    {
      int pos;
      char *nick;

      pos = xo->pos;
      nick = bit_pool[pos].nick;
      fprintf (fw, "PRIVMSG root :allow %s\r\n", nick);
      fflush (fw);

      return XO_FOOT;
    }
  return XO_FOOT;
}

static int
bit_block (xo)
     XO *xo;
{

  if (vans ("確定要封鎖他 (Y/N)？[N]") == 'y')
    {
      int pos;
      char *nick;

      pos = xo->pos;
      nick = bit_pool[pos].nick;
      fprintf (fw, "PRIVMSG root :block %s\r\n", nick);
      fflush (fw);

      return XO_FOOT;
    }
  return XO_FOOT;
}

#if 0
static int
bit_save (xo)
     XO *xo;
{

  vmsg ("設定好別人暱稱後存起來，重新上站後不用再設定一次 :p");
  vmsg ("不過我還沒寫好啦 ^^;;");
  return XO_FOOT;
}
#endif

static int
bit_remv (xo)
     XO *xo;
{
  int pos;
  char *nick;

  pos = xo->pos;
  nick = bit_pool[pos].nick;

  if (vans ("確定要刪除好友 (Y/N)？[N]") == 'y')
    {
      fprintf (fw, "PRIVMSG root :remove %s\r\n", nick);
      fflush (fw);
    }
  return XO_INIT;

}
static int
bit_onick (xo)
     XO *xo;
{
  int pos;
  char *nick, str[10];

  pos = xo->pos;
  nick = bit_pool[pos].nick;

  vmsg ("暫時只能取英文");
  move (b_lines - 1, 0);
  clrtoeol ();

  if (!vget (b_lines, 0, "幫他取個新名字吧：", str, 10, DOECHO))
    return XO_FOOT;
  if (strchr (str, ';') || strchr (str, ','))
    return XO_FOOT;

  fprintf (fw, "PRIVMSG root :rename %s %s\r\n", nick, str);
  fflush (fw);

  bit_fgets ();
  if (sock <=0)
    return XO_QUIT;

  while (!strstr ((buf), "Nick"))
    {
      bit_fgets ();
      if (sock <=0)
	return XO_QUIT;

    };

  sleep (1);

  return XO_INIT;
}

static int
bit_mynick ()
{
  char nick[40];

  if (!vget (b_lines, 0, "我的新暱稱：", nick, 38, DOECHO))
    return XO_FOOT;
  if (strchr (nick, ';') || strchr (nick, ','))
    return XO_FOOT;

  fprintf (fw, "PRIVMSG root :nick 0 %s\r\n", nick);
  fflush (fw);

  return XO_FOOT;
}

static int
bit_add ()
{

  char addr[40];

  if (!vget (b_lines, 0, "輸入新增好友信箱：", addr, 38, DOECHO))
    return XO_FOOT;

  fprintf (fw, "PRIVMSG root :add 0 %s\r\n", addr);
  fflush (fw);

  return XO_INIT;
}

/* static */ int
bit_recall ()
{
  char fpath[80];

  usr_fpath (fpath, cuser.userid, FN_MSN);
//  more (fpath, (char *) -1);
more (fpath, NULL);

  return XO_INIT;
}

void
bit_abort ()
{
  if (sock >0)
    {
      fprintf (fw, "QUIT :bye\r\n");
      fflush (fw);
      fclose (fw);
      fclose (fr);
      sock = 0;
    }
}

static int
bit_close ()
{

  bit_abort ();

  zmsg ("請稍候 ..... ");
  sleep (1);
  vmsg ("連線中斷！");

  return XO_QUIT;
}

static int
bit_test ()
{

  char smiler_buf[32];
  sprintf(smiler_buf,"%d",cutmp->pid);
  vmsg (smiler_buf);
  return XO_INIT;
}

static KeyFunc bit_cb[] = {
  XO_INIT, bit_init,
  XO_LOAD, bit_load,
  XO_HEAD, bit_head,
  XO_BODY, bit_body,

  'b', bit_allow,
  'B', bit_block,
  'a', bit_add,
  'd', bit_remv,
  'l', bit_recall,
//  's', bit_save,
//  'n', bit_onick,
  'c', bit_mynick,
  'w', bit_write,
  Ctrl ('K'), bit_close,
  'h', bit_help
};

void
bit_rqst ()
{
  char *nick, *msg, send[100], file[128];
  FILE *fp;

  while (fgets (buf, sizeof (buf), fr))
    {
      if (msg = strstr (buf, "PRIVMSG"))
	{
	  msg = strstr (msg, ":");
	  *msg++;
	  nick = strtok (buf, "!");
	  *nick++;
	  sprintf (send, "\033[1;33;46m★%s (@msn) \033[37;45m %s \033[m", nick, msg);

	  usr_fpath (file, cuser.userid, FN_MSN);
	  fp = fopen (file, "a");
	  fprintf (fp, "\033[1;33;46m★%s (@msn) \033[m：%s", nick, msg);
	  fclose (fp);

	  cursor_save ();
	  outz (send);
	  cursor_restore ();
	  refresh ();
	  bell ();
	  break;
	}
    }
}


int
bit_main ()
{

  int i = 0;
  char *tmp, account[50], pass[30];

  if (sock <= 0)
    {

      if (!vget (b_lines - 1, 0, "輸入 msn 帳號：", account, 45, DOECHO))
	return 0;

      if (!vget (b_lines - 1, 0, "輸入密碼：", pass, 25, NOECHO))
	return 0;

      move (b_lines - 1, 0);
      sleep (1);
      clrtoeol ();
      sock = dns_open ("127.0.0.1", 6667);
      zmsg ("連接中 :p");
      sleep (1);

      if (sock > 0)
	{
	  zmsg
	    ("登入中，快了別急，本來登入就要等一下咩 :p (想像小綠人在轉 ^^O)");

	  fr = fdopen (sock, "r");
	  fw = fdopen (sock, "w");

	  fprintf (fw, "NICK %d\r\n", cutmp->pid);
	  fflush (fw);
	  fprintf (fw, "USER bitlbee ono ccy :bitlbee run\r\n");
	  fflush (fw);
	  fprintf (fw, "JOIN #bitlbee\r\n");
	  fflush (fw);

	  fprintf (fw, "PRIVMSG root :account add msn %s %s\r\n", account,
	    pass);
	  fflush (fw);
	  fprintf (fw, "PRIVMSG root :account on\r\n");
	  fflush (fw);
	  fprintf (fw, "PRIVMSG root :set charset BIG-5\r\n");
	  fflush (fw);


	  sleep (10);

	  /* 前面 login 有 9 行 configure, 略過 */
	  while (i < 13)
	    {
	      bit_fgets ();
	      if (sock <=0)
		return 0;

	      i++;
	    };

	  /* root 再設定 mode, 略過 */
	  while (bit_fgets ())
	    {
	      if (sock <=0)
		return XO_QUIT;

	      tmp = strstr (buf, "Error");

	      if (tmp)
		{
		  sock = 0;
		  vmsg ("帳號或密碼輸入錯誤喔 :p");
		  goto login_error;
		}
	    };
	}
    }

  if (sock > 0)
    {
      xz[XZ_BITLBEE - XO_ZONE].xo = &bit_xo;
      xz[XZ_BITLBEE - XO_ZONE].cb = bit_cb;
      if (i)
	bell ();
      xover (XZ_BITLBEE);
    }
  else
    vmsg ("無法開啟連線，請至 sysop 版回報");

login_error:
  return 0;
}
