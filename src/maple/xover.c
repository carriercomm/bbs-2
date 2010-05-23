/*-------------------------------------------------------*/
/* xover.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : board/mail interactive reading routines 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef MY_FAVORITE
#define MSG_ZONE_SWITCH	"�ֳt�����G(A)��� (B)�峹 (C)�ݪO (F)�̷R (M)�H�� (U)�ϥΪ� (W)���y�G"
#else
#define MSG_ZONE_SWITCH	"�ֳt�����G(A)��� (B)�峹 (C)�ݪO (M)�H�� (U)�ϥΪ� (W)���y�G"
#endif


/* ----------------------------------------------------- */
/* keep xover record					 */
/* ----------------------------------------------------- */


static XO *xo_root;		/* root of overview list */


XO *
xo_new(path)
  char *path;
{
  XO *xo;
  int len;

  len = strlen(path) + 1;

  xo = (XO *) malloc(sizeof(XO) + len);

  memcpy(xo->dir, path, len);

  return xo;
}


XO *
xo_get(path)
  char *path;
{
  XO *xo;

  for (xo = xo_root; xo; xo = xo->nxt)
  {
    if (!strcmp(xo->dir, path))
      return xo;
  }

  xo = xo_new(path);
  xo->nxt = xo_root;
  xo_root = xo;
  xo->xyz = NULL;
  xo->pos = XO_TAIL;		/* �Ĥ@���i�J�ɡA�N��Щ�b�̫᭱ */

  return xo;
}


#ifdef AUTO_JUMPPOST
XO *
xo_get_post(path, brd)		/* itoc.010910: �Ѧ� xover.c xo_get()�A�� XoPost �q�����y */
  char *path;
  BRD *brd;
{
  XO *xo;
  HDR hdr;
  int fd;
  int pos, locus, mid;	/* locus:������ mid:������ pos:�k���� */

  for (xo = xo_root; xo; xo = xo->nxt)
  {
    if (!strcmp(xo->dir, path))
      return xo;
  }

  xo = xo_new(path);
  xo->nxt = xo_root;
  xo_root = xo;
  xo->xyz = NULL;

  /* �|����s brd->blast �� �̫�@�g�wŪ �� �u���@�g�A�h��Ъ�����̫� */
  if (brd->btime < 0 || !brh_unread(brd->blast) || 
    (pos = rec_num(path, sizeof(HDR))) <= 1 || (fd = open(path, O_RDONLY)) < 0)
  {
    xo->pos = XO_TAIL;	/* ��Щ�b�̫᭱ */
    return xo;
  }

  /* ��Ĥ@�g��Ū binary search */
  pos--;
  locus = 0;
  while (1)
  {
    if (pos <= locus + 1)
      break;

    mid = locus + ((pos - locus) >> 1);
    lseek(fd, (off_t) (sizeof(HDR) * mid), SEEK_SET);
    if (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (brh_unread(BMAX(hdr.chrono, hdr.stamp)))
	pos = mid;
      else
	locus = mid;
    }
    else
    {
      break;
    }
  }

  /* �S��: �p�G�k���а��d�b 1�A���G�إi��A�@�O��Ū��ĤG�g�A�t�@���s�Ĥ@�g���SŪ */
  if (pos == 1)
  {
    /* �ˬd�Ĥ@�g�O�_�wŪ */
    lseek(fd, (off_t) 0, SEEK_SET);
    if (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (brh_unread(BMAX(hdr.chrono, hdr.stamp)))	/* �Y�s�Ĥ@�g�]��Ū�Apos �զ^�h�Ĥ@�g */
	pos = 0;
    }
  }

  close(fd);
  xo->pos = pos;	/* �Ĥ@���i�J�ɡA�N��Щ�b�Ĥ@�g��Ū */

  return xo;
}
#endif


#if 0
void
xo_free(xo)
  XO *xo;
{
  char *ptr;

  if (ptr = xo->xyz)
    free(ptr);
  free(xo);
}
#endif


/* ----------------------------------------------------- */
/* interactive menu routines			 	 */
/* ----------------------------------------------------- */


char xo_pool[(T_LINES - 4) * XO_RSIZ];	/* XO's data I/O pool */


void
xo_load(xo, recsiz)
  XO *xo;
  int recsiz;
{
  int fd, max;

  max = 0;
  if ((fd = open(xo->dir, O_RDONLY)) >= 0)
  {
    int pos, top;
    struct stat st;

    fstat(fd, &st);
    max = st.st_size / recsiz;
    if (max > 0)
    {
      pos = xo->pos;
      if (pos <= 0)
      {
	pos = top = 0;
      }
      else
      {
	top = max - 1;
	if (pos > top)
	  pos = top;
	top = (pos / XO_TALL) * XO_TALL;
      }
      xo->pos = pos;
      xo->top = top;

      lseek(fd, (off_t) (recsiz * top), SEEK_SET);
      read(fd, xo_pool, recsiz * XO_TALL);
    }
    close(fd);
  }

  xo->max = max;
}


int					/* XO_LOAD:�R��  XO_FOOT:���� */
xo_rangedel(xo, size, fchk, fdel)	/* itoc.031001: �Ϭq�R�� */
  XO *xo;
  int size;
  int (*fchk) ();			/* �ˬd�Ϭq���O�_���Q�O�@���O��  0:�R�� 1:�O�@ */
  void (*fdel) ();			/* ���F�R���O���H�~�A�٭n���Ǥ���� */
{
  char ans[8];
  int head, tail;

  vget(b_lines, 0, "[�]�w�R���d��] �_�I�G", ans, 6, DOECHO);
  head = atoi(ans);
  if (head <= 0)
  {
    zmsg("�_�I���~");
    return XO_FOOT;
  }

  vget(b_lines, 28, "���I�G", ans, 6, DOECHO);
  tail = atoi(ans);
  if (tail < head)
  {
    zmsg("���I���~");
    return XO_FOOT;
  }

  if (vget(b_lines, 41, msg_sure_ny, ans, 3, LCECHO) == 'y')
  {
    int fd, total;
    char *data, *phead, *ptail;
    struct stat st;

    if ((fd = open(xo->dir, O_RDONLY)) < 0)
      return XO_FOOT;

    fstat(fd, &st);
    total = st.st_size;
    head = (head - 1) * size;
    tail = tail * size;
    if (head > total)
    {
      close(fd);
      return XO_FOOT;
    }
    if (tail > total)
      tail = total;

    data = (char *) malloc(total);
    read(fd, data, total);
    close(fd);

    total -= tail;
    phead = data + head;
    ptail = data + tail;

    if (fchk || fdel)
    {
      char *ptr;
      for (ptr = phead; ptr < ptail; ptr += size)
      {
	if (fchk && fchk(ptr))		/* �o���O���O�@���Q�� */
	{
	  memcpy(phead, ptr, size);
	  phead += size;
	  head += size;
	}
	else if (fdel)			/* ���F�R���O���A�٭n�� fdel() */
	{
	  fdel(xo, ptr);
	}
      }
    }

    memcpy(phead, ptail, total);

    if ((fd = open(xo->dir, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
    {
      write(fd, data, total + head);
      close(fd);
    }

    free(data);
    return XO_LOAD;
  }
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* Tag List ����					 */
/* ----------------------------------------------------- */


int TagNum;			/* tag's number */
TagItem TagList[TAG_MAX];	/* ascending list */


int
Tagger(chrono, recno, op)
  time4_t chrono;
  int recno;
  int op;			/* op : TAG_NIN / TOGGLE / INSERT */
/* ----------------------------------------------------- */
/* return 0 : not found	/ full				 */
/* 1 : add						 */
/* -1 : remove						 */
/* ----------------------------------------------------- */
{
  int head, tail, pos, cmp;
  TagItem *tagp;

  for (head = 0, tail = TagNum - 1, tagp = TagList, cmp = 1; head <= tail;)
  {
    pos = (head + tail) >> 1;
    cmp = tagp[pos].chrono - chrono;
    if (!cmp)
    {
      break;
    }
    else if (cmp < 0)
    {
      head = pos + 1;
    }
    else
    {
      tail = pos - 1;
    }
  }

  if (op == TAG_NIN)
  {
    if (!cmp && recno)		/* �����Y�ԡG�s recno �@�_��� */
      cmp = recno - tagp[pos].recno;
    return cmp;
  }

  tail = TagNum;

  if (!cmp)
  {
    if (op != TAG_TOGGLE)
      return 0;

    TagNum = --tail;
    memcpy(&tagp[pos], &tagp[pos + 1], (tail - pos) * sizeof(TagItem));
    return -1;
  }

  if (tail < TAG_MAX)
  {
    TagItem buf[TAG_MAX];

    TagNum = tail + 1;
    tail = (tail - head) * sizeof(TagItem);
    tagp += head;
    memcpy(buf, tagp, tail);
    tagp->chrono = chrono;
    tagp->recno = recno;
    memcpy(++tagp, buf, tail);
    return 1;
  }

  /* TagList is full */

  bell();
  return 0;
}


void
EnumTag(data, dir, locus, size)
  void *data;
  char *dir;
  int locus;
  int size;
{
  rec_get(dir, data, size, TagList[locus].recno);
}


int
AskTag(msg)
  char *msg;
/* ----------------------------------------------------- */
/* return value :					 */
/* -1	: ����						 */
/* 0	: single article				 */
/* o.w.	: whole tag list				 */
/* ----------------------------------------------------- */
{
  char buf[80];
  int num;

  num = TagNum;

  if (num)	/* itoc.020130: �� TagNum �~�� */
  {  
    sprintf(buf, "�� %s A)��g�峹 T)�аO�峹 Q)���}�H[%c] ", msg, num ? 'T' : 'A');
    switch (vans(buf))
    {
    case 'q':
      return -1;

    case 'a':
      return 0;
    }
  }
  return num;
}


/* ----------------------------------------------------- */
/* tag articles according to title / author		 */
/* ----------------------------------------------------- */


static int
xo_tag(xo, op)
  XO *xo;
  int op;
{
  int fsize, count;
  char *token, *fimage;
  HDR *head, *tail;

  fimage = f_map(xo->dir, &fsize);
  if (fimage == (char *) -1)
    return XO_NONE;

  head = (HDR *) xo_pool + (xo->pos - xo->top);
  if (op == Ctrl('A'))
  {
    token = head->owner;
    op = 0;
  }
  else
  {
    token = str_ttl(head->title);
    op = 1;
  }

  head = (HDR *) fimage;
  tail = (HDR *) (fimage + fsize);

  count = 0;

  do
  {
    if (!strcmp(token, op ? str_ttl(head->title) : head->owner))
    {
      if (!Tagger(head->chrono, count, TAG_INSERT))
	break;
    }
    count++;
  } while (++head < tail);

  munmap(fimage, fsize);
  return XO_BODY;
}


int					/* XO_LOAD:�R��  XO_FOOT/XO_NONE:���� */
xo_prune(xo, size, fvfy, fdel)		/* itoc.031003: ���ҧR�� */
  XO *xo;
  int size;
  int (*fvfy) ();			/* �ˬd�Ϭq���O�_���Q�O�@���O��  0:�R�� 1:�O�@ */
  void (*fdel) ();			/* ���F�R���O���H�~�A�٭n���Ǥ���� */
{
  int fd, total, pos;
  char *data, *phead, *ptail, *ptr;
  char buf[80];
  struct stat st;

  if (!TagNum)
    return XO_NONE;

  sprintf(buf, "�T�w�n�R�� %d �g���Ҷ�(Y/N)�H[N] ", TagNum);
  if (vans(buf) != 'y')
    return XO_FOOT;

  if ((fd = open(xo->dir, O_RDONLY)) < 0)
    return XO_FOOT;

  fstat(fd, &st);
  data = (char *) malloc(total = st.st_size);
  total = read(fd, data, total);
  close(fd);

  phead = data;
  ptail = data + total;
  pos = 0;
  total = 0;

  for (ptr = phead; ptr < ptail; ptr += size)
  {
    if (fvfy(ptr, pos))			/* �o���O���O�@���Q�� */
    {
      memcpy(phead, ptr, size);
      phead += size;
      total += size;
    }
    else if (fdel)			/* ���F�R���O���A�٭n�� fdel() */
    {
      fdel(xo, ptr);
    }
    pos++;
  }

  if ((fd = open(xo->dir, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
  {
    write(fd, data, total);
    close(fd);
  }

  free(data);

  TagNum = 0;

  return XO_LOAD;
}


/* ----------------------------------------------------- */
/* Tag's batch operation routines			 */
/* ----------------------------------------------------- */


extern BCACHE *bshm;    /* lkchu.981229 */


static int
xo_tbf(xo)
  XO *xo;
{
  char fpath[128], *dir;
  HDR *hdr, xhdr;
  int tag, locus, xmode;
  FILE *fp;

  if (!cuser.userlevel)
    return XO_NONE;

  tag = AskTag("������Ȧs��");
  if (tag < 0)
    return XO_FOOT;

  if (!(fp = tbf_open()))
    return XO_FOOT;

  hdr = tag ? &xhdr : (HDR *) xo_pool + xo->pos - xo->top;

  locus = 0;
  dir = xo->dir;

  do
  {
    if (tag)
    {
      fputs(str_line, fp);
      EnumTag(hdr, dir, locus, sizeof(HDR));
    }

    xmode = hdr->xmode;

    /* itoc.000319: �ץ�����Ť峹���o�פJ�Ȧs�� */
    /* itoc.010602: GEM_RESTRICT �M POST_RESTRICT �ǰt�A�ҥH�[�K�峹�]���o�פJ�Ȧs�� */
    if (xmode & (GEM_RESTRICT | GEM_RESERVED))
      continue;

    if ((*dir == 'b') && !chkrescofo(hdr))
      continue;

    if (!(xmode & GEM_FOLDER))		/* �d hdr �O�_ plain text */
    {
      hdr_fpath(fpath, dir, hdr);
      f_suck(fp, fpath);
    }
  } while (++locus < tag);

  fclose(fp);
  zmsg("��������");

  return XO_FOOT;
}


static int
xo_forward(xo)
  XO *xo;
{
  static char rcpt[64];
  char fpath[64], folder[64], *dir, *title, *userid;
  HDR *hdr, xhdr;
  int tag, locus, userno, cc, xmode;
  int method;		/* ��H�� 0:���~ >0:�ۤv <0:��L�����ϥΪ� */

  if (!cuser.userlevel || HAS_PERM(PERM_DENYMAIL))
    return XO_NONE;

  if (xo->dir[0] == 'b')
  {
    if ((currbattr & BRD_NOFORWARD) && !(bbstate & STAT_BM))
    {
      vmsg("���ݪO�T�����");
      return XO_NONE;
    }
  }

  tag = AskTag("��H");
  if (tag < 0)
    return XO_FOOT;

  if (!rcpt[0])
    strcpy(rcpt, cuser.email);

  if (!vget(b_lines, 0, "�ت��a�G", rcpt, sizeof(rcpt), GCARRY))
    return XO_FOOT;

  userid = cuser.userid;
  userno = 0;

  if (!mail_external(rcpt))	/* ���~�d�I */
  {
    if (!str_cmp(rcpt, userid))
    {
      /* userno = cuser.userno; */	/* Thor.981027: �H��ﶰ���ۤv���q���ۤv */
      method = 1;
    }
    else
    {
      if (!HAS_PERM(PERM_LOCAL))
	return XO_FOOT;

      if ((userno = acct_userno(rcpt)) <= 0)
      {
	zmsg(err_uid);
	return XO_FOOT;
      }
      method = -1;
    }

    usr_fpath(folder, rcpt, fn_dir);
  }
  else
  {
    if (!HAS_PERM(PERM_INTERNET))
    {
      vmsg("�z�L�k�H�H�쯸�~");
      return XO_FOOT;
    }

    if (not_addr(rcpt))
    {
      zmsg(err_email);
      return XO_FOOT;
    }

    method = 0;
  }

  hdr = tag ? &xhdr : (HDR *) xo_pool + xo->pos - xo->top;

  dir = xo->dir;
  title = hdr->title;
  locus = 0;
  cc = -1;

  char str_tag_score[50];

  do
  {
    if (tag)
      EnumTag(hdr, dir, locus, sizeof(HDR));

    xmode = hdr->xmode;

    /* itoc.000319: �ץ�����Ť峹���o��H */
    /* itoc.010602: GEM_RESTRICT �M POST_RESTRICT �ǰt�A�ҥH�[�K�峹�]���o��H */
    if (xmode & (GEM_RESTRICT | GEM_RESERVED))
      continue;

    if (xmode & POST_NOFORWARD)		/* ���g�峹�T�� */
      continue;

    if (!(xmode & GEM_FOLDER))		/* �d hdr �O�_ plain text */
    {
      hdr_fpath(fpath, dir, hdr);

      if (method)		/* ��H���� */
      {
	HDR mhdr;

	if ((cc = hdr_stamp(folder, HDR_COPY, &mhdr, fpath)) < 0)
	  break;

	if (method > 0)		/* ��H�ۤv */
	{
	  strcpy(mhdr.owner, "[�� �� ��]");
	  mhdr.xmode = MAIL_READ | MAIL_NOREPLY;
	  sprintf(str_tag_score," ����� %s ��bbs�H�c ",cuser.userid);
	}
	else			/* ��H��L�ϥΪ� */
	{
	  strcpy(mhdr.owner, userid);
	  sprintf(str_tag_score," ����� %s ��bbs�H�c ",rcpt);
	}
	strcpy(mhdr.nick, cuser.username);
	strcpy(mhdr.title, title);
	if ((cc = rec_add(folder, &mhdr, sizeof(HDR))) < 0)
	  break;
      }
      else			/* ��H���~ */
      {
	if ((cc = bsmtp(fpath, title, rcpt, 0)) < 0)
	  break;
	sprintf(str_tag_score," ��H�ܯ��~ ");
      }
    }
    if (xo->dir[0] == 'b')
    {
      if (currbattr & BRD_SHOWTURN)
	post_t_score(xo,str_tag_score,hdr);
    }
  } while (++locus < tag);

  if (userno > 0 && cc >= 0)
    m_biff(userno);

  zmsg(cc >= 0 ? msg_sent_ok : "�����H��L�k�H�F");

  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* �峹�@�̬d�ߡB�v���]�w				 */
/* ----------------------------------------------------- */


int
xo_uquery(xo)
  XO *xo;
{
  HDR *hdr;
  char *userid;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  userid = hdr->owner;
  if (strchr(userid, '.'))
    return XO_NONE;

  move(1, 0);
  clrtobot();
  my_query(userid, NULL);
  return XO_HEAD;
}


int
xo_usetup(xo)
  XO *xo;
{
  HDR *hdr;
  char *userid;
  ACCT acct;

  if (!HAS_PERM(PERM_ALLACCT))
    return XO_NONE;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  userid = hdr->owner;
  if (strchr(userid, '.') || (acct_load(&acct, userid) < 0))
    return XO_NONE;

  if (!adm_check())
    return XO_FOOT;

  move(3, 0);
  acct_setup(&acct, 1);
  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* �D�D���\Ū						 */
/* ----------------------------------------------------- */


#define RS_TITLE	0x001	/* author/title */
#define RS_FORWARD      0x002	/* backward */
#define RS_RELATED      0x004
#define RS_FIRST	0x008	/* find first article */
#define RS_CURRENT      0x010	/* match current read article */
#define RS_THREAD	0x020	/* search the first article */
#define RS_SEQUENT	0x040	/* sequential read */
#define RS_MARKED 	0x080	/* marked article */
#define RS_UNREAD 	0x100	/* unread article */
#define	RS_BOARD	0x1000	/* �Ω� RS_UNREAD�A��e�������i���| */

#define CURSOR_FIRST	(RS_RELATED | RS_TITLE | RS_FIRST)
#define CURSOR_NEXT	(RS_RELATED | RS_TITLE | RS_FORWARD)
#define CURSOR_PREV	(RS_RELATED | RS_TITLE)
#define RELATE_FIRST	(RS_RELATED | RS_TITLE | RS_FIRST | RS_CURRENT)
#define RELATE_NEXT	(RS_RELATED | RS_TITLE | RS_FORWARD | RS_CURRENT)
#define RELATE_PREV	(RS_RELATED | RS_TITLE | RS_CURRENT)
#define THREAD_NEXT	(RS_THREAD | RS_FORWARD)
#define THREAD_PREV	(RS_THREAD)

/* Thor: �e���mark�峹, ��K���D��������D���B�z */

#define MARK_NEXT	(RS_MARKED | RS_FORWARD | RS_CURRENT)
#define MARK_PREV	(RS_MARKED | RS_CURRENT)


typedef struct
{
  int key;			/* key stroke */
  int map;			/* the mapped threading op-code */
}      KeyMap;


static KeyMap keymap[] =
{
  /* search title / author */

  '?', RS_TITLE | RS_FORWARD,
  '|', RS_TITLE,
  'A', RS_FORWARD,
  'Q', 0,

  /* thread : currtitle */

  '[', RS_RELATED | RS_TITLE | RS_CURRENT,
  ']', RS_RELATED | RS_TITLE | RS_FORWARD | RS_CURRENT,
  '=', RS_RELATED | RS_TITLE | RS_FIRST | RS_CURRENT,

  /* i.e. < > : make life easier */

  ',', RS_THREAD,
  '.', RS_THREAD | RS_FORWARD,

  /* thread : cursor */

  '-', RS_RELATED | RS_TITLE,
  '+', RS_RELATED | RS_TITLE | RS_FORWARD,
  '\\', RS_RELATED | RS_TITLE | RS_FIRST,

  /* Thor: marked : cursor */
  '\'', RS_MARKED | RS_FORWARD | RS_CURRENT,
  ';', RS_MARKED | RS_CURRENT,

  /* Thor: �V�e��Ĥ@�g��Ū���峹 */
  /* Thor.980909: �V�e�䭺�g��Ū, �Υ��g�wŪ */
  '`', RS_UNREAD /* | RS_FIRST */,

  /* sequential */

  ' ', RS_SEQUENT | RS_FORWARD,
  KEY_RIGHT, RS_SEQUENT | RS_FORWARD,
  KEY_PGDN, RS_SEQUENT | RS_FORWARD,
  KEY_DOWN, RS_SEQUENT | RS_FORWARD,
  /* Thor.990208: ���F��K�ݤ峹�L�{��, ���ܤU�g, ���M�W�h�Qxover�Y���F:p */
  'j', RS_SEQUENT | RS_FORWARD,

  KEY_UP, RS_SEQUENT,
  KEY_PGUP, RS_SEQUENT,
  /* Thor.990208: ���F��K�ݤ峹�L�{��, ���ܤW�g, ���M�W�h�Qxover�Y���F:p */
  'k', RS_SEQUENT,

  /* end of keymap */

  (char) NULL, -1
};


static int
xo_keymap(key)
  int key;
{
  KeyMap *km;
  int ch;

  km = keymap;
  while (ch = km->key)
  {
    if (ch == key)
      break;
    km++;
  }
  return km->map;
}


/* itoc.010913: xo_thread() �^�ǭ�                */
/*  XO_NONE: �S���δN�O��ЩҦb�A���βM b_lines */
/*  XO_FOOT: �S���δN�O��ЩҦb�A�ݭn�M b_lines */
/*  XO_BODY: ���F�A���b�O��                     */
/* -XO_NONE: ���F�A�N�b�����A���βM b_lines     */
/* -XO_FOOT: ���F�A�N�b�����A�ݭn�M b_lines     */


static int
xo_thread(xo, op)
  XO *xo;
  int op;
{
  static char s_author[16], s_title[32], s_unread[2] = "0";
  char buf[80];

  char *tag, *query, *title;
  const int origpos = xo->pos, origtop = xo->top, max = xo->max;
  int pos, match, near, neartop;	/* Thor: neartop �P near ����� */
  int top, bottom, step, len;
  HDR *pool, *hdr;

  match = XO_NONE;
  pos = origpos;
  top = origtop;
  pool = (HDR *) xo_pool;
  hdr = pool + (pos - top);
  near = 0;
  step = (op & RS_FORWARD) - 1;		/* (op & RS_FORWARD) ? 1 : -1 */

  if (op & RS_RELATED)
  {
    tag = hdr->title;
    if (op & RS_CURRENT)
    {
      query = currtitle;
      if (op & RS_FIRST)
      {
	if (!strcmp(query, tag))	/* �ثe���N�O�Ĥ@���F */
	  return XO_NONE;
	near = -1;
      }
    }
    else
    {
      title = str_ttl(tag);
      if (op & RS_FIRST)
      {
	if (title == tag)
	  return XO_NONE;
	near = -1;
      }
      strcpy(query = buf, title);
    }
  }
  else if (op & RS_UNREAD)
  {
    /* Thor.980909: �߰� "���g��Ū" �� "���g�wŪ" */

    near = xo->dir[0];
    if (near != 'b' && near != 'u')	/* itoc.010913: �u���\�b�ݪO/�H�c�j�M */
      return XO_NONE;			/* itoc.040916.bug: �S������b�H�c��ذϷj�M */

    if (!vget(b_lines, 0, "�V�e��M 0)���g��Ū 1)���g�wŪ ", s_unread, sizeof(s_unread), GCARRY))
      return XO_FOOT;

    if (*s_unread == '0')
      op |= RS_FIRST;

    if (near == 'b')		/* search board */
      op |= RS_BOARD;

    near = -1;
  }
  else if (!(op & (RS_THREAD | RS_SEQUENT | RS_MARKED)))
  {
    if (op & RS_TITLE)
    {
      title = "���D";
      tag = s_title;
      len = sizeof(s_title);
    }
    else
    {
      title = "�@��";
      tag = s_author;
      len = sizeof(s_author);
    }

    sprintf(query = buf, "�j�M%s(%s)�G", title, (step > 0) ? "��" : "��");
    if (!vget(b_lines, 0, query, tag, len, GCARRY))
      return XO_FOOT;

    str_lowest(query, tag);
  }

  bottom = top + XO_TALL;
  if (bottom > max)
    bottom = max;

  for (;;)
  {
    if (step > 0)
    {
      if (++pos >= max)
	break;
    }
    else
    {
      if (--pos < 0)
	break;
    }

    /* buffer I/O : shift sliding window scope */

    if (pos < top || pos >= bottom)
    {
      xo->pos = pos;
      xo_load(xo, sizeof(HDR));

      top = xo->top;
      bottom = top + XO_TALL;
      if (bottom > max)
	bottom = max;

      hdr = pool + (pos - top);
    }
    else
    {
      hdr += step;
    }

#ifdef HAVE_REFUSEMARK
    if (!chkrestrict(hdr))
      continue;
#endif

    if (op & RS_SEQUENT)
    {
      match = -1;
      break;
    }

    /* Thor: �e�� search marked �峹 */

    if (op & RS_MARKED)
    {
      if (hdr->xmode & POST_MARKED)
      {
	match = -1;
	break;
      }
      continue;
    }

    /* �V�e��M��M��Ū/�wŪ�峹 */

    if (op & RS_UNREAD)
    {
#define UNREAD_FUNC()  (op & RS_BOARD ? brh_unread(BMAX(hdr->chrono, hdr->stamp)) : !(hdr->xmode & MAIL_READ))

      if (op & RS_FIRST)	/* ���g��Ū */
      {
	if (UNREAD_FUNC())
	{
	  near = pos;
	  neartop = top;
	  continue;
	}
      }
      else			/* ���g�wŪ */
      {
	if (!UNREAD_FUNC())
	{
	  match = -1;
	  break;
	}
      }
      continue;
    }

    /* ------------------------------------------------- */
    /* �H�U�j�M title / author				 */
    /* ------------------------------------------------- */

    if (op & (RS_TITLE | RS_THREAD))
    {
      title = hdr->title;	/* title ���V [title] field */
      tag = str_ttl(title);	/* tag ���V thread's subject */

      if (op & RS_THREAD)
      {
	if (tag == title)
	{
	  match = -1;
	  break;
	}
	continue;
      }
    }
    else
    {
      tag = hdr->owner;	/* tag ���V [owner] field */
    }

    if (((op & RS_RELATED) && !strncmp(tag, query, 40)) ||
      (!(op & RS_RELATED) && str_sub(tag, query)))
    {
      if ((op & RS_FIRST) && tag != title)
      {
	near = pos;		/* �O�U�̱���_�I����m */
	neartop = top;
	continue;
      }

      match = -1;
      break;
    }
  }

  /* top = xo->top = buffering �� top */
  /* �p�G match = -1 ���ܧ��F�A�� pos, top = �n�h���a�� */
  /* �p�G RS_FIRST && near >= 0 ���ܧ��F�A�� near, neartop = �n�h���a�� */

#define CLEAR_FOOT()	(!(op & RS_RELATED) && ((op & RS_UNREAD) || !(op & (RS_THREAD | RS_SEQUENT | RS_MARKED))))

  if (match < 0)			/* ���F */
  {
    xo->pos = pos;			/* ��n�h����m��i�h */

    if (top != origtop)			/* �b�O�����F */
      match = XO_BODY;
    else				/* �b�������F */
      match = CLEAR_FOOT() ? -XO_FOOT : -XO_NONE;
  }
  else if ((op & RS_FIRST) && near >= 0)/* ���F */
  {
    xo->pos = near;			/* ��n�h����m��i�h */

    /* �ѩ�O RS_FIRST ��Ĥ@�g�A�ҥH buffering �� top �i�����̫ᵲ�G neartop ��e�� */
    if (top != neartop)
      xo_load(xo, sizeof(HDR));

    if (xo->top != origtop)		/* �b�O�����F */
      match = XO_BODY;
    else				/* �b�������F */
      match = CLEAR_FOOT() ? -XO_FOOT : -XO_NONE;
  }
  else					/* �䤣�� */
  {
    xo->pos = origpos;			/* �٭��Ӧ�m */

    if (top != origtop)			/* �^�ثe�Ҧb�� */
      xo_load(xo, sizeof(HDR));

    match = CLEAR_FOOT() ? XO_FOOT : XO_NONE;
  }

  return match;
}


/* Thor.990204: ���Ҽ{more �Ǧ^��, �H�K�ݤ@�b�i�H�� []...
                ch �����emore()���ҫ���key */   
int
xo_getch(xo, ch)
  XO *xo;
  int ch;
{
  int op;

  if (!ch)
    ch = vkey();

  op = xo_keymap(ch);
  if (op >= 0)
  {
    ch = xo_thread(xo, op);
    if (ch != XO_NONE)
      ch = XO_BODY;		/* �~���s�� */
  }

  return ch;
}


/* ----------------------------------------------------- */
/* XZ							 */
/* ----------------------------------------------------- */

#ifndef NEW_KeyFunc
extern KeyFunc pal_cb[];
extern KeyFunc f_pal_cb[];
extern KeyFunc bmw_cb[];
extern KeyFunc post_cb[];
#else
extern NewKeyFunc bmw_cb[];
extern NewKeyFunc class_cb[];
extern NewKeyFunc mf_cb[];
extern NewKeyFunc gem_cb[];
extern NewKeyFunc mbox_cb[];
extern NewKeyFunc xmbox_cb[];
extern NewKeyFunc pal_cb[];
extern NewKeyFunc f_pal_cb[];
extern NewKeyFunc post_cb[];
extern NewKeyFunc news_cb[];
extern NewKeyFunc xpost_cb[];
extern NewKeyFunc ulist_cb[];


//NewKeyFunc aloha_cb[];
//NewKeyFunc bit_cb[];
//NewKeyFunc nbrd_cb[];
//NewKeyFunc rss_cb[];
//NewKeyFunc song_cb[];
//NewKeyFunc vote_cb[];

typedef struct
{
  NewKeyFunc *cb;
} MY_XZ;

static MY_XZ my_xz[] =
{
  bmw_cb,    // maple/bmw.c
  class_cb,  // maple/board.c
  mf_cb,     // maple/favor.c
  gem_cb,    // maple/gem.c
  mbox_cb,   // maple/mail.c
  xmbox_cb,  // maple/mail.c
  pal_cb,    // maple/pal.c
  f_pal_cb,  // maple/pal.c
  post_cb,   // maple/post.c
  news_cb,   // maple/post.c
  xpost_cb,  // maple/post.c
  ulist_cb   // maple/ulist.c

//  aloha_cb,  // so/aloha.c
//  bit_cb,    // so/bitlbee.c
//  nbrd_cb,   // so/newbrd.c
//  rss_cb,    // so/rss.c
//  song_cb,   // so/song.c
//  vote_cb    // so/vote.c

};

static int key_in_xover[] =
{
  KEY_LEFT,  //  0
  'q',       //  1
  KEY_UP,    //  2
  'k',       //  3
  KEY_DOWN,  //  4
  'j',       //  5
  ' ',       //  6
  KEY_PGDN,  //  7
  'N',       //  8
  KEY_PGUP,  //  9
  'P',       // 10
  KEY_HOME,  // 11
  '0',       // 12
  KEY_END,   // 13
  '$',       // 14
  '1',       // 15
  '2',       // 16
  '3',       // 17
  '4',       // 18
  '5',       // 19
  '6',       // 20
  '7',       // 21
  '8',       // 22
  '9',       // 23
  KEY_RIGHT, // 24
  '\n',      // 25
  Ctrl('Z'), // 26
  Ctrl('U'), // 27
  Ctrl('W'), // 28

  /* �H�U�� zone >= XZ_XPOST */

  'C',       // 29
  'F',       // 30
  Ctrl('C'), // 31
  Ctrl('A'), // 32
  Ctrl('T')  // 33

};

#endif

XZ xz[] =
{
  {NULL, NULL, M_BOARD, FEETER_CLASS},	/* XZ_CLASS */
  {NULL, NULL, M_LUSERS, FEETER_ULIST},	/* XZ_ULIST */
  {NULL, pal_cb, M_PAL, FEETER_PAL},	/* XZ_PAL */
  {NULL, NULL, M_PAL, FEETER_ALOHA},	/* XZ_ALOHA */
  {NULL, NULL, M_VOTE, FEETER_VOTE},	/* XZ_VOTE */
  {NULL, bmw_cb, M_BMW, FEETER_BMW},	/* XZ_BMW */
  {NULL, NULL, M_MF, FEETER_MF},	/* XZ_MF */
  {NULL, NULL, M_COSIGN, FEETER_COSIGN},/* XZ_COSIGN */
  {NULL, NULL, M_SONG, FEETER_SONG},	/* XZ_SONG */
  {NULL, NULL, M_READA, FEETER_NEWS},	/* XZ_NEWS */
  {NULL, NULL, M_MSN, FEETER_BITLBEE},  /* XZ_BITLBEE */
  {NULL, f_pal_cb, M_PAL, FEETER_FAKE_PAL},/* XZ_FAKE_PAL */
  {NULL, NULL, M_RSS, FEETER_RSS},       /* XZ_RSS */

  /* smiler.090519: �H�U�����P hdr �����A��l�Щ�b�W�� */
  /*                include/modes.h �O�o�P�B�վ�        */

  {NULL, NULL, M_READA, FEETER_XPOST},	/* XZ_XPOST */
  {NULL, NULL, M_RMAIL, FEETER_MBOX},	/* XZ_MBOX */
  {NULL, post_cb, M_READA, FEETER_POST},/* XZ_POST */
  {NULL, NULL, M_GEM, FEETER_GEM}	/* XZ_GEM */
};


static int
xo_jump(pos, zone)
  int pos;			/* ���ʴ�Ш� number �Ҧb���S�w��m */
  int zone;			/* itoc.010403: �� zone �]�Ƕi�� */
{
  char buf[6];

  buf[0] = pos;
  buf[1] = '\0';
  vget(b_lines, 0, "���ܲĴX���G", buf, sizeof(buf), GCARRY);

#if 0
  move(b_lines, 0);
  clrtoeol();
#endif
  outf(xz[zone - XO_ZONE].feeter);	/* itoc.010403: �� b_lines ��W feeter */

  pos = atoi(buf);

  if (pos > 0)
    return XO_MOVE + pos - 1;

  return XO_NONE;
}


/* ----------------------------------------------------- */
/* XOX browser						 */
/* ----------------------------------------------------- */

#define XOX_UP		0x001
#define XOX_DOWN	0x002
#define XOX_LEFT	0x004
#define XOX_RIGHT	0x008

#define MAX_XOX_X	3
#define MAX_XOX_Y	20

static screenline xox_slt[T_LINES];
static int xox_x_roll;


typedef struct
{
  char name[16];
  int (*func) ();
  int key;
} XOX;

typedef struct
{
  int value;
  char name[5];
} XOX_KEY;

//typedef struct
//{
//  int key;
//  char key_name[6];    /* smiler.090606: ��M�� xox_key[]         */
//  char func_name[24];  /* smiler.090606: ��M�� NewKeyFunc struct */
//} XOX_HELP_ITEM;

static XOX_KEY xox_key[] = 
{
  0x00, "  ",
  0x01, "^A",
  0x02, "^B",
  0x03, "^C",
  0x04, "^D",
  0x05, "^E",
  0x06, "^F",
  0x07, "^G",
  //0x08, "^H",
  0x08, "Bksp",		/* smiler.090606: ^H �P Backspace ���ơA����� */
  //0x09, "^I",
  0x09, "Tab",		/* smiler.090606: ^I �P Tab ���ơA����� */
  //0x0a, "^J",
  0x0a, "Enter",	/* smiler.090606: ^J �P Enter ���ơA����� */
  0x0b, "^K",
  0x0c, "^L",
  0x0d, "^M",
  0x0e, "^N",
  0x0f, "^O",
  
  0x10, "^P",
  0x11, "^Q",
  0x12, "^R",
  0x13, "^S",
  0x14, "^T",
  0x15, "^U",
  0x16, "^V",
  0x17, "^W",
  0x18, "^X",
  0x19, "^Y",
  0x1a, "^Z",
  0x1b, "Esc",
  0x1c, "  ",
  0x1d, "  ",
  0x1e, "  ",
  0x1f, "  ",

  0x20, " ",
  0x21, "!",
  0x22, "\"",
  0x23, "#",
  0x24, "$",
  0x25, "%",
  0x26, "&",
  0x27, "\'",
  0x28, "(",
  0x29, ")",
  0x2a, "*",
  0x2b, "+",
  0x2c, ",",
  0x2d, "-",
  0x2e, ".",
  0x2f, "/",

  0x30, "0",
  0x31, "1",
  0x32, "2",
  0x33, "3",
  0x34, "4",
  0x35, "5",
  0x36, "6",
  0x37, "7",
  0x38, "8",
  0x39, "9",
  0x3a, ":",
  0x3b, ";",
  0x3c, "<",
  0x3d, "=",
  0x3e, ">",
  0x3f, "?",

  0x40, "@",
  0x41, "A",
  0x42, "B",
  0x43, "C",
  0x44, "D",
  0x45, "E",
  0x46, "F",
  0x47, "G",
  0x48, "H",
  0x49, "I",
  0x4a, "J",
  0x4b, "K",
  0x4c, "L",
  0x4d, "M",
  0x4e, "N",
  0x4f, "O",

  0x50, "P",
  0x51, "Q",
  0x52, "R",
  0x53, "S",
  0x54, "T",
  0x55, "U",
  0x56, "V",
  0x57, "W",
  0x58, "X",
  0x59, "Y",
  0x5a, "Z",
  0x5b, "[",
  0x5c, "\\",
  0x5d, "]",
  0x5e, "^",
  0x5f, "_",

  0x60, "`",
  0x61, "a",
  0x62, "b",
  0x63, "c",
  0x64, "d",
  0x65, "e",
  0x66, "f",
  0x67, "g",
  0x68, "h",
  0x69, "i",
  0x6a, "j",
  0x6b, "k",
  0x6c, "l",
  0x6d, "m",
  0x6e, "n",
  0x6f, "o",

  0x70, "p",
  0x71, "q",
  0x72, "r",
  0x73, "s",
  0x74, "t",
  0x75, "u",
  0x76, "v",
  0x77, "w",
  0x78, "x",
  0x79, "y",
  0x7a, "z",
  0x7b, "{",
  0x7c, "|",
  0x7d, "}",
  0x7e, "~",
//  0x7f, "  "

 /* �Ȱ� */

 0xffffffea, "Ins",   /* 16*8 -1 = 127 */
 0xffffffe9, "Del",
 0xffffffeb, "Home",
 0xffffffe8, "End",
 0xffffffe7, "PgUp",
 0xffffffe6, "PgDn",

 0xffffffff, "��",
 0xfffffffe, "��",
 0xfffffffd, "��",
 0xfffffffc, "��"

 /* ���� */

// 0x08, "Bksp",        /* 137 */
// 0x09, "Tab",
// 0x0a, "Enter",
// 0x0d, "\r"
};

char *
xox_key_search(int value)
{
  int i=0;

  if (value>=0 && value < 127)
    return xox_key[value].name;
  else if (value < 0)
  {
    for (i=128; i<137; i++)
    {
      if (value == xox_key[i].value)
	return xox_key[value].name;
    }
  }

  /* smiler.090608: �i��O�� XO_DL �A���s��@�M */

  value = value & (~XO_DL);

  if (value>=0 && value < 127)
    return xox_key[value].name;
  else if (value < 0)
  {
    for (i=128; i<137; i++)
    {
      if (value == xox_key[i].value)
	return xox_key[value].name;
    }
    return xox_key[0].name;
  }
  else
    return xox_key[0].name;
}


XOX_list(xcmd, cmd ,xo)
  NewKeyFunc *xcmd;
  int cmd;
  XO *xo;
{
  int num = cmd | XO_DL; /* Thor.990220: for dynamic load */
  int pos;

  NewKeyFunc *cb;

  for (;;)
  {
    pos = cb->key;
#if 1
     /* Thor.990220: dynamic load , with key | XO_DL */
    if (pos == num)
    {
      void *p = DL_get((char *) cb->func);
      if (p)
      {
	  cb->func = p;
	  pos = cb->key = cmd;
      }
      else
      {
	  cmd = XO_NONE;
	  break;
      }
    }
#endif
    if (pos == cmd)
    {
      cmd = (*(cb->func)) (xo);

      if (cmd == XO_QUIT)
	return;

      break;
    }

    if (pos == 'h')	/* itoc.001029: 'h' �O�@�S�ҡA�N�� *_cb ������ */
    {
      cmd = XO_NONE;	/* itoc.001029: �N���䤣�� call-back, ���@�F! */
      break;
    }

    cb++;
  }

}

void
xox_help(xcmd, xo)
  NewKeyFunc *xcmd;
  XO *xo;
{
  int xox_help_item[20];
  NewKeyFunc *cb, *tail;
  char buf[64];
  char xox_line[900];
  int i=0;
  int j=0;
  int x=0;
  int old_x=0;
  int cmd=0;
  int tmp=0;
  int renew=0;
  int xox_help_len=0;
  int xox_max_len=0;	/* smiler.090606: �[�t�d�� */

  cb = xcmd;

  cb = cb - 1;

  for (i = 0; i<20; i++)
  {
    cb = cb + 1;

    while (cb->level == 'n' && cb->key != 'h')
      cb = cb + 1;

    xox_help_item[i] = (int) (cb - xcmd);
    sprintf(buf, "\033[m %-6.6s %-24.24s\033[m ",  xox_key_search(cb->key), cb->funcname);
    outsxy(buf, 3+i, 30);

    xox_help_len++;

    if (cb->key == 'h')
    {
      tail = cb;

      for (i=i+1; i<20; i++)
      {
	xox_help_item[i] = 0;
	sprintf(buf, "\033[m %-6.6s %-24.24s\033[m ", " ", " ");
	outsxy(buf, 3+i, 30);
      }
      break;
    }
  }

  if (xox_help_len == 20 && strcmp(xox_key_search(cb->key), "h"))
  {

     for (;; i++)
     {
	cb = cb + 1;

	if (cb->key == 'h')
	{
	  tail = cb;
	  break;
	}
     }
     xox_max_len = i + 1;

  }
  else
     xox_max_len = xox_help_len;

  x = 0;
  old_x = 0;

  for (;;)
  {
    old_x = x;

    line_save(3+x, xox_line);
    cb = xcmd + xox_help_item[x];
    sprintf(buf, "\033[m\033[1;33m %-6.6s %-24.24s\033[m ", xox_key_search(cb->key), cb->funcname);
    outsxy(buf, 3+x, 30);

    cmd = vkey();

    switch (cmd)
    {
      case KEY_UP:
	x--;
	break;
      case KEY_DOWN:
	x++;
	break;
      case KEY_PGUP:
	x = x - 20;
	break;
      case KEY_PGDN:
	x = x + 20;
	break;
      case KEY_HOME:
	x = xox_max_len;
	break;
      case KEY_END:
	x = xox_max_len;
	break;
      default:
	return;
    }

    line_restore(3+old_x, xox_line);

    if (xox_max_len <= 20 || (x < xox_help_len && x >= 0))   /* ���ݧ�s��� */
    {
      if (cmd == KEY_PGUP)
	x = old_x;
      else if (cmd == KEY_PGDN)
	x = old_x;
      else if (cmd == KEY_HOME)
	x = 0;
      else if (cmd == KEY_END)
	x = xox_help_len - 1;
      else if (x >= xox_help_len)
	x = x - xox_help_len;
      else if (x < 0)
	x = x + xox_help_len;

      continue;
    }
    else
    {
//      xox_help_len = 0;

      if (cmd == KEY_HOME)
      {
	x = 0;

	cb = xcmd;
	cb = cb - 1;

	for (i = 0; i < 20; i++)
	{
	  cb = cb + 1;
	  while (cb->level == 'n' && cb->key != 'h')
	    cb = cb + 1;

//          xox_help_len++;

	  xox_help_item[i] = (int)(cb - xcmd);

	  if (cb->key == 'h')
	  {
	    for (i=i+1; i<20; i++)
	      xox_help_item[i] = 0;
	  }
	}
      }
      else if (cmd == KEY_END)
      {
	x = 19;
	cb = tail;

	for (i = 0; i<20; i++)
	{
	  xox_help_item[i] = (int)(cb - xcmd);
	  cb = cb - 1;

//          xox_help_len ++;

	  if (cb->level == 'n' || i==19)
	  {
	    x = i;

	    for (j = 0; j <= (i/2); j++)
	    {
	      tmp = xox_help_item[j];
	      xox_help_item[j] = xox_help_item[i - j];
	      xox_help_item[i - j] = tmp; 
	    }
	    for (i = i+1; i<20; i++)
	      xox_help_item[i] = 0;
	  }
	}
      }
      //else if (cmd == KEY_PGUP)
      else if (cmd == KEY_PGDN)
      {
	x = old_x;

	cb = xcmd + xox_help_item[19];

	if (cb->key != 'h')
	{
	  cb = cb + 1;

	  for (i = 1;;)
	  {
	    if (cb->key == 'h' || i==20)
	      break;
	    else
	    {
	      cb = cb + 1;
	      i++;
	    }
	  }

	  for (i = 0; i < 20; i++)
	    xox_help_item[19 - i] = (int)(cb - xcmd) - i;
	}
      }

//    else if (x >= xox_help_len)
//    {
       /* ��M�U�褧�U�@�� */
       //xox_help_tmp
       /* �w�g�O���ݤF�A���^�_�Y */
//    }
//    else if(x < 0)
//    {
       /* ��M�W�褧�W�@�� */
       /* �w�g�O�W�ݤF�A���^���� */
//    }

      for (i=0; i<20; i++)
      {
	cb = xcmd + xox_help_item[i];
	sprintf(buf, "\033[m %-6.6s %-24.24s\033[m ", xox_key_search(cb->key), cb->funcname);
	outsxy(buf, 3+i, 30);
      }

    }
  }

}


void
XOX_test(xcmd, xo)
  NewKeyFunc *xcmd;
  XO *xo;
{
  vmsg("test");
}

int
XOX_browser(xcmd, xo)
  NewKeyFunc *xcmd;
  XO *xo;
{

  NewKeyFunc *cb;
  int x=0;
  int y=0;
  int cus_x=0;   //
  int cus_y=0;   //
  int num_y = 3;
  int num_x = MAX_XOX_X;
  int old_x = 0;
  int old_y = 0;
  int cmd;

  int i=0;

  char buf[64];

  //screenline xox_line;

  char xox_line[900];

  /*
        �Q  �E
        �K  �T
        �C���G
        ���|�@
    y
    |
    ->x
             */

        /*y x */
  XOX xox[][MAX_XOX_X] =
  {
    /* y=0 */
    {
      {"�@", xox_help, XOX_LEFT | XOX_UP | XOX_RIGHT | XOX_DOWN},
      {"�|", XOX_test, XOX_LEFT | XOX_UP | XOX_RIGHT | XOX_DOWN},
      {"��", XOX_test, XOX_LEFT | XOX_UP | XOX_RIGHT | XOX_DOWN}
    },
    /* y=1 */
    {
      {"�G", XOX_test, XOX_UP | XOX_DOWN},
      {"��", XOX_test, XOX_UP | XOX_DOWN},
      {"�C", XOX_test, XOX_UP | XOX_DOWN}
    },
    /* y=2 */
    {
      {"�T", XOX_test, XOX_UP | XOX_DOWN},
      {},
      {"�K", XOX_test, XOX_UP | XOX_DOWN}
    },
    /* y=3 */
    {
      {"�E", XOX_test, XOX_UP | XOX_DOWN},
      {},
      {"�Q", XOX_test, XOX_UP | XOX_DOWN}
    }
  };

  int xox_h[5] = {4,2,4};
  int xox_x[MAX_XOX_X] = {20, 40, 60};

  vs_save(xox_slt);

  num_y = xox_h[x];

  for (i = 0; i < num_y; i++)
  {
    sprintf(buf, "\033[m \033[m\033[1;32m%-16.16s\033[m ", xox[i][x].name);
    outsxy(buf, b_lines - 1 - i, xox_x[x] - 1);
  }

  sprintf(buf, "\033[m \033[m\033[1;32m%-16.16s\033[m ", " ");
  outsxy(buf, b_lines - 1 - i, xox_x[x] - 1);

  line_save(b_lines - 1, xox_line);


  cb = xcmd;

  for (;;)
  {
    if (x >= num_x)
      x = x - num_x;
    else if (x < 0)
      x = x + num_x;

    if (y >= num_y)
      y = y - num_y;
    else if(y < 0)
      y = y + num_y;

    if (y != old_y || x != old_x)
    {
      line_restore(b_lines - 1 - old_y, xox_line);
      line_save(b_lines - 1 - y, xox_line);

      if (x != old_x)
      {
	num_y = xox_h[x];
	vs_restore(xox_slt);

	for (i = 0; i < num_y; i++)
	{
	  sprintf(buf, "\033[m \033[m\033[1;32m%-16.16s\033[m ", xox[i][x].name);
	  outsxy(buf, b_lines - 1 - i, xox_x[x] - 1);
	}

	sprintf(buf, "\033[m \033[m\033[1;32m%-16.16s\033[m ", " ");
	outsxy(buf, b_lines - 1 - i, xox_x[x] - 1);

	line_save(b_lines - 1, xox_line);
      }
    }

    sprintf(buf, "\033[m \033[m\033[1;33m%s\033[m ", xox[y][x].name);
    outsxy(buf, b_lines - 1 - y, xox_x[x] - 1);


    old_x = x;
    old_y = y;

    cmd = vkey();

    switch (cmd)
    {
      case KEY_UP:
	if (xox[y][x].key & XOX_UP)
	  y++;
	break;
      case KEY_DOWN:
	if (xox[y][x].key & XOX_DOWN)
	  y--;
	break;
      case KEY_LEFT:
	if (xox[y][x].key & XOX_LEFT)
	  x--;
	break;
      case KEY_RIGHT:
	if (xox[y][x].key & XOX_RIGHT)
	  x++;
	break;
      case '\n':
	//cb = xcmd + 6;
	//sprintf(buf, "%d %s", cb->key, cb->funcname);
	//vmsg(buf);
	(*(xox[y][x].func)) (xcmd, xo);
	break;
      default:
	break;
    }

    if (cmd == ' ' /*&& x==0 && y==0*/)
      break;
  }

  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* interactive menu routines			 	 */
/* ----------------------------------------------------- */


void
xover(cmd)
  int cmd;
{
  int pos, num, zone, sysmode;
  XO *xo;
#ifndef NEW_KeyFunc
  KeyFunc *xcmd, *cb;
#else
  NewKeyFunc *xcmd, *cb;
#endif

  for (;;)
  {
    while (cmd != XO_NONE)
    {
      if (cmd == XO_FOOT)
      {
	outf(xz[zone - XO_ZONE].feeter);	/* itoc.010403: �� b_lines ��W feeter */
	break;
      }

      if (cmd >= XO_ZONE)
      {
	/* --------------------------------------------- */
	/* switch zone					 */
	/* --------------------------------------------- */

	zone = cmd;
	cmd -= XO_ZONE;
	xo = xz[cmd].xo;
	xcmd = xz[cmd].cb;
	sysmode = xz[cmd].mode;

	TagNum = 0;		/* clear TagList */
	cmd = XO_INIT;
	utmp_mode(sysmode);
      }
      else if (cmd >= XO_MOVE - XO_TALL)
      {
	/* --------------------------------------------- */
	/* calc cursor pos and show cursor correctly	 */
	/* --------------------------------------------- */

	/* cmd >= XO_MOVE - XO_TALL so ... :chuan: ���] cmd = -1 ?? */

	/* fix cursor's range */

	num = xo->max - 1;

	/* pos = (cmd | XO_WRAP) - (XO_MOVE + XO_WRAP); */
	/* cmd &= XO_WRAP; */
	/* itoc.020124: �ץ��b�Ĥ@���� PGUP�B�̫�@���� PGDN�B�Ĥ@���� UP�B�̫�@���� DOWN �|�� XO_WRAP �X�Ю��� */

	if (cmd > XO_MOVE + (XO_WRAP >> 1))     /* XO_WRAP >> 1 ���j�L�峹�� */
	{
	  pos = cmd - (XO_MOVE + XO_WRAP);
	  cmd = 1;				/* �� KEY_UP �� KEY_DOWN */
	}
	else
	{
	  pos = cmd - XO_MOVE;
	  cmd = 0;
	}

	/* pos: �n���h������  cmd: �O�_�� KEY_UP �� KEY_DOWN */

	if (pos < 0)
	{
	  /* pos = (zone == XZ_POST) ? 0 : num; *//* itoc.000304: �\Ū��Ĥ@�g�� KEY_UP �� KEY_PGUP ���|½��̫� */
	  pos = num;	/* itoc.020124: ½��̫�����K�A���Ӥ��|���H�O���WŪ�� :P */
	}
	else if (pos > num)
	{
	  if (cmd)
	  {
	    pos = 0;
	  }
	  else
	  {
	    if (zone == XZ_POST)
	      pos = num;	/* itoc.000304: �\Ū��̫�@�g�� KEY_DOWN �� KEY_PGDN ���|½��̫e */
	    else
	      pos = (cmd || pos == num + XO_TALL) ? 0 : num;	/* itoc.020124: �n�קK�p�G�b�˼ƲĤG���� KEY_PGDN�A
								   �ӳ̫�@���g�ƤӤַ|�������h�Ĥ@���A�ϥΪ̷|
								   �����D���̫�@���A�G���b�̫�@�����@�U */
	  }
	}

	/* check cursor's range */

	cmd = xo->pos;

	if (cmd == pos)
	  break;

	xo->pos = pos;
	num = xo->top;
	if ((pos < num) || (pos >= num + XO_TALL))
	{
	  xo->top = (pos / XO_TALL) * XO_TALL;
	  move(3, 0);		/* �ѨM���D���Ʈ�, KEY_PGUP �e���ݯd���D */
	  clrtobot();
	  cmd = XO_LOAD;	/* ���J��ƨä��H��� */
	}
	else
	{
	  move(3 + cmd - num, 0);
#ifdef HAVE_LIGHTBAR
	  /* verit.030129 : xover ���� */
	  if (cuser.ufo & UFO_LIGHTBAR && xcmd[0].key == XO_ITEM)
	  {
	    int tmp = xo->pos;
	    clrtoeol();
	    xo->pos = cmd - num + xo->top;	/* (xo->pos - xo->top + xo->top) == xo->pos ? */
	    (*(xcmd[0].func)) (xo, 0);
	    xo->pos = tmp;
	  }
	  else
#endif
	  outc(' ');

	  break;		/* �u���ʴ�� */
	}
      }

      /* ----------------------------------------------- */
      /* ���� call-back routines			 */
      /* ----------------------------------------------- */

      cb = xcmd;
      num = cmd | XO_DL; /* Thor.990220: for dynamic load */
      for (;;)
      {
	pos = cb->key;
#if 1
	/* Thor.990220: dynamic load , with key | XO_DL */
	if (pos == num)
	{
	  void *p = DL_get((char *) cb->func);
	  if (p) 
	  {
	    cb->func = p;
	    pos = cb->key = cmd;
	  }
	  else
	  {
	    cmd = XO_NONE;
	    break;
	  }
	}
#endif
	if (pos == cmd)
	{
	  cmd = (*(cb->func)) (xo);

	  if (cmd == XO_QUIT)
	    return;

	  break;
	}
	
	if (pos == 'h')		/* itoc.001029: 'h' �O�@�S�ҡA�N�� *_cb ������ */
	{
	  cmd = XO_NONE;	/* itoc.001029: �N���䤣�� call-back, ���@�F! */
	  break;
	}

	cb++;
      }

    } /* Thor.990220.����: end of while (cmd!=XO_NONE) */

    utmp_mode(sysmode); 
    /* Thor.990220:����:�ΨӦ^�_ event handle routine �^�ӫ᪺�Ҧ� */

    pos = xo->pos;

    if (xo->max > 0)		/* Thor: �Y�O�L�F��N��show�F */
    {
      num = 3 + pos - xo->top;
      move(num, 0);
#ifdef HAVE_LIGHTBAR
      /* verit.20030129 : xover ���� */
      if (cuser.ufo & UFO_LIGHTBAR && xcmd[0].key == XO_ITEM)
      {
	clrtoeol();
	(*(xcmd[0].func)) (xo, 1);
      }
      else
#endif
      outc('>');
    }

    cmd = vkey();

    /* itoc.����: �H�U�w�q�F�򥻫���A�ҿװ򥻫���A�N�O���ʪ��������A�q�Ω�Ҧ� XZ_ ���a�� */  

    /* ------------------------------------------------- */
    /* �򥻪���в��� routines				 */
    /* ------------------------------------------------- */

#ifndef NEW_KeyFunc
    if (cmd == KEY_LEFT || (cmd == 'q'))
#else
    //if (cmd == KEY_LEFT || (cmd == 'q'))
    if (cmd == key_in_xover[0] || (cmd == key_in_xover[1]))
#endif
    {
      TagNum = 0;	/* itoc.050413: �q��ذϦ^��峹�C���ɭn�M�� tag */
      return;
    }
    else if (xo->max <= 0)	/* Thor: �L�F��h�L�k����� */
    {
      continue;
    }
#ifndef NEW_KeyFunc
    else if (cmd == KEY_UP || cmd == 'k')
#else
    //else if (cmd == KEY_UP || cmd == 'k')
    else if (cmd == key_in_xover[2] || cmd == key_in_xover[3])
#endif
    {
      cmd = pos - 1 + XO_MOVE + XO_WRAP;
    }
#ifndef NEW_KeyFunc
    else if (cmd == KEY_DOWN || cmd == 'j')
#else
    //else if (cmd == KEY_DOWN || cmd == 'j')
    else if (cmd == key_in_xover[4] || cmd == key_in_xover[5])
#endif
    {
      cmd = pos + 1 + XO_MOVE + XO_WRAP;
    }
#ifndef NEW_KeyFunc
    else if (cmd == ' ' || cmd == KEY_PGDN || cmd == 'N')
#else
    //else if (cmd == ' ' || cmd == KEY_PGDN || cmd == 'N')
    else if (cmd == key_in_xover[6] || cmd == key_in_xover[7] || cmd == key_in_xover[8])
#endif
    {
      cmd = pos + XO_TALL + XO_MOVE;
    }
#ifndef NEW_KeyFunc
    else if (cmd == KEY_PGUP || cmd == 'P')
#else
    //else if (cmd == KEY_PGUP || cmd == 'P')
    else if (cmd == key_in_xover[9] || cmd == key_in_xover[10])
#endif
    {
      cmd = pos - XO_TALL + XO_MOVE;
    }
#ifndef NEW_KeyFunc
    else if (cmd == KEY_HOME || cmd == '0')
#else
    //else if (cmd == KEY_HOME || cmd == '0')
    else if (cmd == key_in_xover[11] || cmd == key_in_xover[12])
#endif
    {
      cmd = XO_MOVE;
    }
#ifndef NEW_KeyFunc
    else if (cmd == KEY_END || cmd == '$')
#else
    //else if (cmd == KEY_END || cmd == '$')
    else if (cmd == key_in_xover[13] || cmd == key_in_xover[14])
#endif
    {
      if (zone == XZ_POST)
      {
	int pb = last_nobottom(xo->dir);
	if (xo->pos == pb)
	  cmd = xo->max - 1 + XO_MOVE;
	else
	  cmd = pb + XO_MOVE;
      }
      else
	cmd = xo->max - 1 + XO_MOVE;
    }
#ifndef NEW_KeyFunc
    else if (cmd >= '1' && cmd <= '9')
#else
    //else if (cmd >= '1' && cmd <= '9')
    else if (cmd >= key_in_xover[15] && cmd <= key_in_xover[16])
#endif
    {
      cmd = xo_jump(cmd, zone);
    }
#ifndef NEW_KeyFunc
    else if (cmd == KEY_RIGHT || cmd == '\n')
#else
    //else if (cmd == KEY_RIGHT || cmd == '\n')
    else if (cmd == key_in_xover[24] || cmd == key_in_xover[25])
#endif
    {
      cmd = 'r';
    }
#ifdef NEW_KeyFunc
    else if (cmd == Ctrl('H'))
    {
      cmd = XOX_browser(xcmd, xo);
    }
#endif

    /* ------------------------------------------------- */
    /* switch Zone					 */
    /* ------------------------------------------------- */

#ifdef  EVERY_Z

#ifndef NEW_KeyFunc
    else if (cmd == Ctrl('Z'))
#else
    //else if (cmd == Ctrl('Z'))
    else if (cmd == key_in_xover[26])
#endif
    {
      cmd = every_Z(zone);
    }
#ifndef NEW_KeyFunc
    else if (cmd == Ctrl('U'))
#else
    //else if (cmd == Ctrl('U'))
    else if (cmd == key_in_xover[27])
#endif
    {
      cmd = every_U(zone);
    }
#ifndef NEW_KeyFunc
    else if (cmd == Ctrl('W'))
#else
    //else if (cmd == Ctrl('W'))
    else if (cmd == key_in_xover[28])
#endif
    {
      DL_func("bin/dictd.so:main_dictd");
      cmd = XO_INIT;
    }

#endif

    /* ------------------------------------------------- */
    /* ��l������					 */
    /* ------------------------------------------------- */

    else
    {
      if (zone >= XZ_XPOST)		/* xo_pool ���񪺬O HDR */
      {
	/* --------------------------------------------- */
	/* Tag						 */
	/* --------------------------------------------- */

#ifndef NEW_KeyFunc
	if (cmd == 'C')
#else
	//if (cmd == 'C')
	if (cmd == key_in_xover[29])
#endif
	{
	  cmd = xo_tbf(xo);
	}
#ifndef NEW_KeyFunc
	else if (cmd == 'F')
#else
	//else if (cmd == 'F')
	else if (cmd == key_in_xover[30])
#endif
	{
	  cmd = xo_forward(xo);
	}
#ifndef NEW_KeyFunc
	else if (cmd == Ctrl('C'))
#else
	//else if (cmd == Ctrl('C'))
	else if (cmd == key_in_xover[31])
#endif
	{
	  if (TagNum)
	  {
	    TagNum = 0;
	    cmd = XO_BODY;
	  }
	  else
	    cmd = XO_NONE;
	}
#ifndef NEW_KeyFunc
	else if (cmd == Ctrl('A') || cmd == Ctrl('T'))
#else
	//else if (cmd == Ctrl('A') || cmd == Ctrl('T'))
	else if (cmd == key_in_xover[32] || cmd == key_in_xover[33])
#endif
	{
	  cmd = xo_tag(xo, cmd);
	}

	/* --------------------------------------------- */
	/* �D�D���\Ū					 */
	/* --------------------------------------------- */

	if (zone == XZ_XPOST)		/* �걵�����䴩�D�D���\Ū */
	  continue;

	pos = xo_keymap(cmd);
	if (pos >= 0)			/* �p�G���O����V�� */
	{
	  cmd = xo_thread(xo, pos);	/* �h�d�d�O���@�� thread �j�M */	  

	  if (cmd < 0)		/* �b������� match */
	  {
	    move(num, 0);
#ifdef HAVE_LIGHTBAR
	    /* verit.030129 : xover ���� */
	    if (cuser.ufo & UFO_LIGHTBAR && xcmd[0].key == XO_ITEM)
	    {
	      int tmp = xo->pos;
	      clrtoeol();
	      xo->pos = num + xo->top - 3;
	      (*(xcmd[0].func)) (xo, 0);
	      xo->pos = tmp;
	    }
	    else
#endif
	    outc(' ');
	    /* cmd = XO_NONE; */
	    /* itoc.010913: �Y�Ƿj�M�n�� b_lines ��W feeter */
	    cmd = -cmd;
	  }
	}
      }

      /* ----------------------------------------------- */
      /* ��L���浹 call-back routine �h�B�z		 */
      /* ----------------------------------------------- */

    } /* Thor.990220.����: end of vkey() handling */
  }
}


/* ----------------------------------------------------- */
/* Thor.980725: ctrl Z everywhere			 */
/* ----------------------------------------------------- */


#ifdef EVERY_Z
int z_status = 0;	/* �i�J�X�h */

int
every_Z(zone)
  int zone;				/* �ǤJ�Ҧb XZ_ZONE�A�Y�ǤJ 0�A���ܤ��b xover() �� */
{
  int cmd, tmpbno, tmpmode;
  int tmpstate;		/* smiler.070602: every_z ��ݪO��,�i�Ȧs�ݪO�v��(bbsstate) */

  /* itoc.000319: �̦h every_Z �@�h */
  if (z_status >= 1)
    return XO_NONE;
  else
    z_status++;

  cmd = zone;

  outz(MSG_ZONE_SWITCH);

  tmpbno = vkey();	/* �ɥ� tmpbno �Ӵ����p�g */
  if (tmpbno >= 'A' && tmpbno <= 'Z')
    tmpbno |= 0x20;
  switch (tmpbno)
  {
  case 'a':
    cmd = XZ_GEM;
    break;

  case 'b':
    if (currbno >= 0)	/* �Y�w��w�ݪO�A�i�J�ݪO�A�_�h��ݪO�C�� */
    {
      cmd = XZ_POST;
      break;
    }

  case 'c':
    cmd = XZ_CLASS;
    break;

#ifdef MY_FAVORITE
  case 'f':
    if (cuser.userlevel)
      cmd = XZ_MF;
    break;
#endif

  case 'm':
    if (cuser.userlevel)
      cmd = XZ_MBOX;
    break;

  case 'u':
    cmd = XZ_ULIST;
    break;

  case 'w':
    if (cuser.userlevel)
      cmd = XZ_BMW;
    break;
  }

  if (cmd == zone)		/* �M�ثe�Ҧb zone �@�ˡA�Ψ��� */
  {
    z_status--;
    return XO_FOOT;		/* �Y�b xover() �������I�s every_Z() �h�e�^ XO_FOOT �Y�i��ø */
  }

  if (cmd == XZ_POST)
    XoPost(currbno);

#ifdef MY_FAVORITE
  if (zone == XZ_POST && (cmd == XZ_CLASS || cmd == XZ_MF))
#else
  if (zone == XZ_POST && cmd == XZ_CLASS)
#endif
    tmpbno = currbno;
  else
    tmpbno = -1;

  tmpmode = bbsmode;
  tmpstate = bbstate;	/* smiler.070602: every_z��,���Ȧs�ثe�ݪO�v�� */
  xover(cmd);

  if (tmpbno >= 0)		/* itoc.030731: ���i��i�J�O���O�A�N�ݭn���s XoPost�A�|�A�ݤ@���i�O�e�� */
    XoPost(tmpbno);

  bbstate = tmpstate;	/* smiler.070602: every_z�^�ӫ�,���^���e�Ȧs���ݪO�v��,�קK�ݪO�v���Q�\�� */
  utmp_mode (tmpmode);  

  z_status--;
  return XO_INIT;		/* �ݭn���s���J xo_pool�A�Y�b xover() ���]�i�Ǧ���ø */
}


int
every_U(zone)
  int zone;			/* �ǤJ�Ҧb XZ_ZONE�A�Y�ǤJ 0�A���ܤ��b xover() �� */
{
  /* itoc.000319: �̦h every_Z �@�h */
  if (z_status >= 1)
    return XO_NONE;

  if (zone != XZ_ULIST)
  {
    int tmpmode;

    z_status++;
    tmpmode = bbsmode;
    xover(XZ_ULIST);
    utmp_mode(tmpmode);
    z_status--;
  }
  return XO_INIT;
}
#endif


/* ----------------------------------------------------- */
/* �� XZ_* ���c����в���				 */
/* ----------------------------------------------------- */


/* �ǤJ: ch, pagemax, num, pageno, cur, redraw */
/* �ǥX: ch, pageno, cur, redraw */
int
xo_cursor(ch, pagemax, num, pageno, cur, redraw)
  int ch, pagemax, num;
  int *pageno, *cur, *redraw;
{
  switch (ch)
  {
  case KEY_LEFT:
  case 'q':
    return 'q';

  case KEY_PGUP:
    if (pagemax != 0)
    {
      if (*pageno)
      {
	(*pageno)--;
      }
      else
      {
	*pageno = pagemax;
	*cur = num % XO_TALL;
      }
      *redraw = 1;
    }
    break;

  case KEY_PGDN:
    if (pagemax != 0)
    {
      if (*pageno == pagemax)
      {
	/* �b�̫�@�����@�U */
	if (*cur != num % XO_TALL)
	{
	  *cur = num % XO_TALL;
	}
	else
	{
	  *pageno = 0;
	  *cur = 0;
	}
      }
      else
      {
	(*pageno)++;
	if (*pageno == pagemax && *cur > num % XO_TALL)
	  *cur = num % XO_TALL;
      }
      *redraw = 1;
    }
    break;

  case KEY_UP:
  case 'k':
    if (*cur == 0)
    {
      if (*pageno != 0)
      {
	*cur = XO_TALL - 1;
	*pageno = *pageno - 1;
      }
      else
      {
	*cur = num % XO_TALL;
	*pageno = pagemax;
      }
      *redraw = 1;
    }
    else
    {
      move(3 + *cur, 0);
      outc(' ');
      (*cur)--;
      move(3 + *cur, 0);
      outc('>');
    }
    break;

  case KEY_DOWN:
  case 'j':
    if (*cur == XO_TALL - 1)
    {
      *cur = 0;
      *pageno = (*pageno == pagemax) ? 0 : *pageno + 1;
      *redraw = 1;
    }
    else if (*pageno == pagemax && *cur == num % XO_TALL)
    {
      *cur = 0;
      *pageno = 0;
      *redraw = 1;
    }
    else
    {
      move(3 + *cur, 0);
      outc(' ');
      (*cur)++;
      move(3 + *cur, 0);
      outc('>');
    }
    break;

  case KEY_HOME:
  case '0':
    *pageno = 0;
    *cur = 0;
    *redraw = 1;
    break;

  case KEY_END:
  case '$':
    *pageno = pagemax;
    *cur = num % XO_TALL;
    *redraw = 1;
    break;

  default:
    if (ch >= '1' && ch <= '9')
    {
      int pos;
      char buf[6];

      buf[0] = ch;
      buf[1] = '\0';
      vget(b_lines, 0, "���ܲĴX���G", buf, sizeof(buf), GCARRY);

      pos = atoi(buf);

      if (pos > 0)
      {
	pos--;
	if (pos >num)
	  pos = num;
	*pageno = pos / XO_TALL;
	*cur = pos % XO_TALL;
      }

      *redraw = 1;	/* �N��S�������A�]�n��ø feeter */
    }
  }

  return ch;
}


/* ----------------------------------------------------- */
/* �������						 */
/* ----------------------------------------------------- */


void
xo_help(path)			/* itoc.021122: ������� */
  char *path;
{
  /* itoc.030510: ��� so �̭� */
  DL_func("bin/help.so:vaHelp", path);
}