/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * ʹ�÷�����
 * ���к�����PL/0Դ�����ļ�?
 * �ش��Ƿ�������������
 * �ش��Ƿ�������ֱ�
 * fa.tmp������������
 * fa1.tmp���Դ�ļ�������ж�Ӧ���׵�ַ
 * fa2.tmp�����?
 * fas.tmp������ֱ�
 */

#include <stdio.h>

#include "pl0.h"
#include "string.h"

/* ����ִ��ʱʹ�õ�ջ */
#define stacksize 500

int main()
{
	bool nxtlev[symnum];	/* symnumΪ32, ����32����� */

	printf("Input pl/0 file?   ");
	scanf("%s", fname);     /* �����ļ��� */

	fin = fopen(fname, "r");

	if (fin)	/* ������ļ�������Ok */
	{
		//printf("List object code?(Y/N)");   /* �Ƿ������������� */
		//scanf("%s", fname);
		listswitch = 0; //(fname[0]=='y' || fname[0]=='Y');

		//printf("List symbol table?(Y/N)");  /* �Ƿ�������ֱ� */
		//scanf("%s", fname);
		tableswitch = 1; //(fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();     /* ��ʼ�� */

		err = 0; 			/* �����ѷ��ֵĴ������ */
		cc = cx = ll = 0;	/* cc��ll: getchʹ�õļ�����, cc��ʾ��ǰ�ַ� */
		ch = ' ';			/* �������л�ȡ�ַ���getch ʹ��, ��ʼ��Ϊһ���ո� */

		if(-1 != getsym())
		{
			fa = fopen("fa.tmp", "w"); fas = fopen("fas.tmp", "w");
			/* nxtlev������Ϊ32��bool����, declbegsys������һ��������ʼ�ķ��ż��ϣ�statbegsys������俪ʼ�ķ��ż���
               ����nxtlev��ʹ���3��������6�����Ŀ�ʼ����, ������һ����������true */
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;			/* �ټ�һ����������� . */

			/* 4������, 1: ��ǰ�ֳ������ڲ�, 2: ���ֱ�ǰβָ��
            3: ���������Ǳ�ʶ��3��������6������bool����
			4: param_num��������  */
			if(-1 == block(0, 0, nxtlev, 0))   /* ���ñ������ */
			{
				fclose(fa);
				fclose(fa1);
				fclose(fas);
				fclose(fin);
				printf("\n");
				return 0;
			}
			
			fclose(fa1);
			fclose(fas);

			if (sym != period)	/* ������һ�����Ų���period, �ͱ�ʾû����ȷ���� */
			{
				error(9);
			}

			if (err == 0)		/* ���ֻ��0������, �Ϳ��Ե��ý��ͳ���ʼ������ */
			{
				fa2 = fopen("fa2.tmp", "w");
				listallcode();
				fclose(fa);
				interpret();    /* ���ý���ִ�г��� */
				fclose(fa2);
			}
			else				/* ���򱨴�, �޷��ý��ͳ��������� */
			{
				printf("Errors in pl/0 program");
			}
		}

		fclose(fin);
	}
	else
	{
		printf("Can't open file!\n");
	}

	printf("\n");
	return 0;
}

/*
* ��ʼ��
*/
void init()
{
	int i;

	/* ���õ��ַ����� */
	for (i=0; i<=255; i++)
	{
		ssym[i] = nul;		/* nul = 0 */
	}
	/* ASCII��Χ(0�C31�����ַ�, 32�C126 ����������ڼ������ҵ����ַ�������127���� DELETE ����, ��128������չASCII��ӡ�ַ�) ��˹�256��
    �˴�����14�����ַ��ķ���ֵ ��ASCII��Ϊ���� ���������� */
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym['#'] = neq;
	ssym[';'] = semicolon;

	/* ���ñ���������,������ĸ˳�򣬱����۰���ң�����elseֻ�ܷ���4��λ */
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
    strcpy(&(word[4][0]), "else");	/* ���ӱ�����else */
    strcpy(&(word[5][0]), "end");
    strcpy(&(word[6][0]), "if");
    strcpy(&(word[7][0]), "odd");
    strcpy(&(word[8][0]), "procedure");
    strcpy(&(word[9][0]), "read");
    strcpy(&(word[10][0]), "then");
    strcpy(&(word[11][0]), "var");
    strcpy(&(word[12][0]), "while");
    strcpy(&(word[13][0]), "write");

	/* ���ñ����ַ��� */
	wsym[0] = beginsym;
	wsym[1] = callsym;
	wsym[2] = constsym;
	wsym[3] = dosym;
    wsym[4] = elsesym;
    wsym[5] = endsym;
    wsym[6] = ifsym;
    wsym[7] = oddsym;
    wsym[8] = procsym;
    wsym[9] = readsym;
    wsym[10] = thensym;
    wsym[11] = varsym;
    wsym[12] = whilesym;
    wsym[13] = writesym;
	/* ����ָ������ */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[mov][0]), "mov");

	/* ���÷��ż� */
	for (i=0; i<symnum; i++)	/* symnum = 32������32������(��ȥ��P_CODEָ��) */
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* ����������ʼ���ż� */
	declbegsys[constsym] = true;	/* �������� */
	declbegsys[varsym] = true;		/* �������� */
	declbegsys[procsym] = true;		/* �������� */

	/* ������俪ʼ���ż� */
	statbegsys[beginsym] = true;	/* �������Ŀ�ʼ */
	statbegsys[callsym] = true;		/* ���̵������ */
	statbegsys[ifsym] = true;		/* if������� */
	statbegsys[whilesym] = true;	/* whileѭ����� */

	/* �������ӣ�����ʽ�ұߵĲ��֣���ʼ���ż� */
	facbegsys[ident] = true;		/* ��ʶ�� */
	facbegsys[number] = true;		/* �޷������� */
	facbegsys[lparen] = true;		/* ������ */
}

/*
* ������ʵ�ּ��ϵļ�������
*/
int inset(int e, bool* s)
{
	return s[e];
}

/* sr: ����Ϊ32��bool����, s1: ����һ��������ʼ�ķ��ż��ϣ�
s2: ������俪ʼ�ķ��ż���, n = 32
����sr��ʹ���3��������6�����Ŀ�ʼ����, ������һ�־���true */
int addset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]||s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&(!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sr[i] = s1[i]&&s2[i];
	}
	return 0;
}

/*
*   ��������ӡ����λ�úʹ������
*/
void error(int n)
{
	char space[81];			/* ��ʼ��space, ���ڱ�ʾ������һ��Line����ֵ�λ�� */
	memset(space,32,81);	/* 32��Ӧ���ַ�Ϊ�ո� */
	/* ����ʱ��ǰ�����Ѿ����꣬����cc-1����ֵΪ0��ʾ'\0'����, ��˿ո�ĳ��Ⱦ��ǳ����λ�� */
	space[cc-1]=0; 			
	printf("****%s!%d\n", space, n);
	fprintf(fa1,"****%s!%d\n", space, n);
	switch(n)
	{
		case 1 :printf("����˵���в�������\":=\". \n");break;
		case 2 :printf("����˵���е�\"=\"��Ӧ��������.  \n");break;
		case 3 :printf("ȱ��\"=\". \n");break;
		case 4 :printf("const ��var ��procedure��ӦΪ��ʶ��. \n");break;
		case 5 :printf("©����\",\"������\";\". \n");break;
		case 6 :printf("����˵����ķ��Ų���ȷ(Ӧ�Ǿ��ӵĿ�ʼ��,���̶����)��\n");break;
		case 7 :printf("����˳������ӦΪ[<����˵������>][<����˵������>] [<����˵������>]<���>��\n");break;
		case 8 :printf("�������ڵ���䲿�ֵķ�����ȷ��\n");break;
		case 9 :printf("�����ĩβ�����˾��\".\"��\n");break;
		case 10 :printf("����֮��©����\";\"��\n");break;
		case 11 :printf("��ʶ��δ����.\n");break;
		case 12 :printf("��ֵ�����ӦΪ������\n");break;
		case 13 :printf("ȱ�ٸ�ֵ�� \":=\" ��\n");break;
		case 14 :printf("call ��ӦΪ��ʶ����\n");break;
		case 15 :printf("call ���ʶ������Ӧ�ǹ��̡�\n");break;
		case 16 :printf("ȱ��\"then\"��\n");break;
		case 17 :printf("ȱ��\"end\"��\";\"��\n");break;
		case 18 :printf("do while ��ѭ�����ȱ��do��\n");break;
		case 19 :printf("����ı�Ų���ȷ��\n");break;
		case 20 :printf("ӦΪ��ϵ�������\n");break;
		case 21 :printf("���ʽ�ڵı�ʶ�����Բ������޷���ֵ�Ĺ��̡�\n");break;
		case 22 :printf("���ʽ��©�������š�\n");break;
		case 23 :printf("���ʽ����ַǷ����š�\n");break;
		case 24 :printf("���ʽ�Ŀ�ʼ��Ϊ�Ƿ����ŷ��š�\n");break;
		case 30 :printf("�����������λ����\n");break;
		case 31 :printf("������ַ�Ͻ硣\n");break;
		case 32 :printf("��������������Ƕ������������\n");break;
		case 33 :printf("ȱ�������š�");break;
		case 34 :printf("read��ȱ�������� (\n");break;
		case 35 :printf("read��û��ʶ�������ʶ��δ���������߱�ʶ�����Ǳ���\n");break;
        default :printf("�Ҳ������ִ���\n");
	}

	err++;
}

/*
* ©���ո񣬶�ȡһ���ַ���
*
* ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
*
* ������getsym���á�
*/
int getch()
{
	/* ll��cc ����ʼ��Ϊ0, cc��ʾ��ǰ�ַ�(ch)��λ��
	��������, ˵���ϴδ����ж�����һ���ַ�(��line��)�Ѿ���������, �͵��ٶ�һ�г���  */
	if (cc == ll)
	{
		if (feof(fin))
		{
			printf("program incomplete");
			return -1;
		}
		cc = ll = 0;			/* ���¸�ֵΪ0 */
		printf("%d ", cx);		/* cx, ���������ָ�룬��ʾ���ǵڼ������������ */
		fprintf(fa1,"%d ", cx);
		ch = ' ';				/* ch����Ϊ32�����ڽ����������ѭ�� */
		while (ch != 10)		/* �տ�ʼ32�϶�������10, �����ѭ��, 10�ǻ��з�(��˼�ǰ����ж���) */
		{
			if (EOF == fscanf(fin,"%c", &ch))	/* �����EOF�ļ�������(�൱�ڻس�), ��line�и�λ��Ϊ'\0'��β��������ѭ�� */
			{
				line[ll] = 0;
				break;
			}
			printf("%c", ch);
			fprintf(fa1, "%c", ch);
			line[ll] = ch;
			ll++;
		}
		printf("\n");
		fprintf(fa1, "\n");
	}
	ch = line[cc];
	cc++;
	return 0;
}

/*
* �ʷ���������ȡһ������
*/
int getsym()
{
	int i,j,k;

	while (ch==' ' || ch==10 || ch==13 || ch==9)  /* ���Կո񡢻��С��س���TAB */
	{
		getchdo;			/* ץȡһ���ַ���ch */
	}
	/* 1. ���ֻ�������a..z��ͷ */
	if (ch>='a' && ch<='z')	
	{           
		k = 0;				/* ��¼��ǰ������Ѿ��ж��ٸ��ַ��� */
		do {
			if(k<al)		/* al: ���ŵ���󳤶� */
			{
				a[k] = ch;	/* a: ������ʱ��ȡ��һ����(����) */
				k++;
			}
			getchdo;		/* else��������滹����ĸ����, kȴ�Ѿ�>=al, ������� */
		} while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
		a[k] = 0;			/* �����'\0'��β */
		strcpy(id, a);		/* ��ʱcopy��id��, id: ��ŵ�ǰ��ʶ�� */
		i = 0;
		j = norw-1;			/* norw: �ؼ��ָ���, 14�� */
		do {     			/* ������ǰ�����Ƿ�Ϊ������, �۰���� */
			k = (i+j)/2;
			if (strcmp(id,word[k]) <= 0)
			{
				j = k - 1;
			}
			if (strcmp(id,word[k]) >= 0)
			{
				i = k + 1;
			}
		} while (i <= j);
		if (i-1 > j)		/* ���i��j������2, ˵���Ǳ����� */
		{
			sym = wsym[k];	/* ͨ���������������� */
		}
		else				/* ����ʧ���������� */
		{
			sym = ident;	/* symΪ��ʶ�����ͣ������ֻ�����id�� */
		}
	}
	/* 2. ������ĸ��ͷ, ��ֻ�������ֻ���� */
	else					
	{
		if (ch>='0' && ch<='9')	/* ����Ƿ�Ϊ���֣���0..9��ͷ */
		{           
			k = 0;				/* ��ʾ����������м�λ�� */
			num = 0;			/* ��¼�����ֵ�ֵ */
			sym = number;		/* ���ֵı�ʶ��Ϊnumber */
			do {
				num = 10*num + ch - '0';	/* ͨ���ۼӼ���str2num */
				k++;
				getchdo;					/* �ٶ�һ��, �����������˳�ѭ�� */
			} while (ch>='0' && ch<='9');	/* ��ȡ�����ַ� */
			//k--;							/* ����һ�����ֲ�+1�����ﲻ���ټ�ȥ */
			if (k > nmax)		/* nmax: number�����λ��, Ϊ14 */
			{
				error(30);		/* �������Ϊ30, ���ֹ���!!! */
			}
		}
	/* 3. ������ĸҲ�������֣����Ƿ���(�����ǵ��ַ�, Ҳ������˫�ַ�) */
		else
		{
			/* 3.1 ��⸳ֵ���� */
			if (ch == ':')      	
			{
				getchdo;
				if (ch == '=')		/* ����¶�ȡ����=, ˵�������� := ��ֵ */
				{
					sym = becomes;	/* ��ֵ���ŵı�ʶ */
					getchdo;
				}
				else
				{
					sym = nul;  	/* û�ж���ð�ŵı�ʶ�����ڣ��˴�Ϊ����ʶ��ķ��� */
				}
			}
			else
			{
			/* 3.2 ���С�ڻ�С�ڵ��ڷ��� */
				if (ch == '<')		
				{
					getchdo;
					if (ch == '=')
					{
						sym = leq;	/* С�ڵ��ڷ��ŵı�ʶ */
						getchdo;
					}
					else
					{
						sym = lss;	/* С�ڷ��ŵı�ʶ */
					}
				}
			/* 3.3 �����ڻ���ڵ��ڷ��� */
				else
				{
					if (ch=='>')	
					{
						getchdo;
						if (ch == '=')
						{
							sym = geq;	/* ���ڵ��ڷ��ŵı�ʶ */
							getchdo;
						}
						else
						{
							sym = gtr;	/* ���ڷ��ŵı�ʶ */
						}
					}
			/* 3.4 ʣ�µ�ֻ���ǵ��ַ� */
					else
					{
						/* �����ʼ��ʱ�иõ��ַ�, ��ֵ��Ӧ��ʶ, ����Ϊ��ʼ��ʱ��nul��ʶ */
						sym = ssym[ch];     /* �����Ų�������������ʱ��ȫ�����յ��ַ����Ŵ��� */
						if (sym != period)
						{
							getchdo;
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* �������������
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax)
	{
		printf("Program too long"); /* ������� */
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}


/*
* ���Ե�ǰ�����Ƿ�Ϸ�
*
* ��ĳһ���֣���һ����䣬һ�����ʽ����Ҫ����ʱʱ����ϣ����һ����������ĳ��?
* ���ò��ֵĺ�����ţ���test���������⣬���Ҹ��𵱼�ⲻͨ��ʱ�Ĳ��ȴ�ʩ��
* ��������Ҫ���ʱָ����ǰ��Ҫ�ķ��ż��ϺͲ����õļ��ϣ���֮ǰδ��ɲ��ֵĺ��
* ���ţ����Լ���ⲻͨ��ʱ�Ĵ���š�
*
* s1:   ������Ҫ�ķ���
* s2:   �������������Ҫ�ģ�����Ҫһ�������õļ�?
* n:    �����
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* ����ⲻͨ��ʱ����ͣ��ȡ���ţ�ֱ����������Ҫ�ļ��ϻ򲹾ȵļ���
           �����ų����в�����FIRST��FOLLOW�ķ��� */
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{	
			/* �����FOLLOW��þ䵱��������getsymdo��ȡ��һ������ʼ�µķ���
        	�����FIRST�������ٶ���ֱ�ӿ�ʼ��һ�ֵķ��� */
			getsymdo;
		}
	}
	return 0;
}

/*
* ���������?
*
* lev:    ��ǰ�ֳ������ڲ�
* tx:     ���ֱ�ǰβָ��
* fsys:   ��main������block�ǣ�3������ + 6�����Ŀ�ʼ���� + .���������ǵ�ǰģ���FIRST���ϣ�
* ParamNum: ���β������� 
*/
int block(int lev, int tx, bool* fsys, int ParamNum)
{
	int i;

	int dx;          		/* ���ַ��䵽����Ե�ַ������ڸó������ÿһ��proc���ᶨ��һ����*/
	int tx0;        		/* ������ʼtx�����ֱ������һ�����ֵ�ָ�� */
	int cx0;        		/* ������ʼcx */
	bool nxtlev[symnum];	/* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
							���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿ�?
							���ݸ��¼�����*/

	dx = 3 + ParamNum;				/* ���µ������Σ���̬���������ߵ�ַ������̬�������׵�ַ��������ֵ���ϵ㣩 */
	tx0 = tx;               			/* ��¼�������ֵĳ�ʼλ�� */
	table[tx0-ParamNum].adr = cx;	/* table���λ�ò������������֣�����main����procedure��
										��������ʱ���jmp��code���λ��cx�����������������ִ��ǰ�޸ģ�
										Ȼ���ٸĳ�inte��λ�� */
	gendo(jmp, 0, 0);					/* ���Ⱦ�������������תjmp 0 0, Ϊ��������������������봦�������� */

	if (lev > levmax)
	{
		error(32);
	}

	do 
	{
		if (sym == constsym)    /* �յ������������ţ���ʼ���������� */
		{
			getsymdo;

			constdeclarationdo(&tx, lev, &dx);  /* dx��ֵ�ᱻconstdeclaration�ı䣬ʹ��ָ�� */
			while (sym == comma)
			{
				getsymdo;
				constdeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);   /*©���˶��Ż��߷ֺ�*/
			}
		}

		if (sym == varsym)      /* �յ������������ţ���ʼ����������� */
		{
			getsymdo;

			vardeclarationdo(&tx, lev, &dx);
			while (sym == comma)
			{
				getsymdo;
				vardeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon)
			{
				getsymdo;
			}
			else
			{
				error(5);
			}
		}

		while (sym == procsym) /* �յ������������ţ���ʼ����������� */
		{
			getsymdo;

			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx); /* ��¼�������� */
				getsymdo;
			}
			else
			{
				error(4);   	/* procedure��ӦΪ��ʶ�� */
			}

			save_dx = dx;		/* �ݴ�dx */
			dx = 3;				/* dx���3, ʹ�ô��α������ӹ�����dx=3��λ�ÿ�ʼ��¼ */
			int tx_cp = tx;		/* �ݴ浱ǰproc�����ֱ��ַ�����ڻ���������� */
			if (sym == lparen)
			{
				param_num = 0;
				do {
					getsymdo;
					if(sym == ident)
					{
						vardeclarationdo(&tx, lev+1, &dx); 
						param_num++;
					}
				} while(sym == comma);

				if (sym == rparen)
				{
					getsymdo;
				} 
				else 
				{
					error(55);		/* ���̴����﷨���� */
				}

				table[tx_cp].val = param_num;	/* ��¼���̲���������val */
			}
			dx = save_dx;			/* �ָ�ԭ��dx */

			if (sym == semicolon)	/* procedure�� ��ʶ����Ӧ���и� ; */
			{
				getsymdo;
			}
			else
			{
				error(5);   		/* ©���˷ֺ� */
			}
			
			memcpy(nxtlev, fsys, sizeof(bool)*symnum);	/* fsys: ��ǰģ���FIRST���� */
			nxtlev[semicolon] = true;	/* nxtlev�²��FIRST���ϣ��̳�����һ��� 3������ + 6�����Ŀ�ʼ���� + .�����������һ���� */
			if (-1 == block(lev+1, tx, nxtlev, param_num))	/* �˴�tx���ڲ�����λ��֮��� */
			{
				return -1;  /* �ݹ���� */
			}
			tx = tx_cp; 	/* txҲ�ع鱾�� */

			if(sym == semicolon)
			{
				/* <�ֳ���> ::= [<����˵��>][<����˵��>][<����˵��>]<���>  ע�������<���>ֻ����һ��
				������˵������������7��<���>���������ٶ���һ��procedure */
				getsymdo;
				memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
				nxtlev[ident] = true;	
				nxtlev[procsym] = true;	/* �����������˼��: �����Լ��������ڶ���procedure */
				/* �������õĴ��룬�����Ƕ�<�ֳ���>��������<���>���ֽ���test
				fsys�Ǵ˲��FIRST��3������ + 6�����Ŀ�ʼ���� + .������ */
				testdo(nxtlev, fsys, 6);
			}
			else
			{
				error(5);   /* ©���˷ֺ� */
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));   /* ֱ��û���������� */
	/* �����table[tx0].adr����jmp��code���λ�ã����޸�jmp��a��ʹֱ֮�������������, ��inte���봦 */
	code[table[tx0-ParamNum].adr].a = cx;    	/* ��ʼ���ɵ�ǰ���̴��� */
	/* ����table[tx0] ��ʵ�ǳ���procedure�������������ֱ��е�ʵ��, adr��ֱ��ָ��inte��code�е�λ��, ��Ȼ��ʱcx��ָ��Ŀգ�����������վͻ���inte */
	table[tx0-ParamNum].adr = cx;	/* ��ǰ���̵���ڵ�ַ */
	/* size����dx: 3+���������� */
	table[tx0-ParamNum].size = dx;	/* ����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size */
	cx0 = cx;
	gendo(inte, 0, dx);             	/* ���ɷ����ڴ���룬dx����size��С */

	if (tableswitch)        			/* ������ֱ� */
	{
		printf("TABLE:\n");
		if (tx0+1-ParamNum > tx)
		{
			printf("    NULL\n");
		}
		for (i=tx0+1-ParamNum; i<=tx; i++)
		{
			switch (table[i].kind)
			{
			case constant:
				printf("    %d const %s ", i, table[i].name);
				printf("val=%d\n", table[i].val);
				fprintf(fas, "    %d const %s ", i, table[i].name);
				fprintf(fas, "val=%d\n", table[i].val);
				break;
			case variable:
				printf("    %d var   %s ", i, table[i].name);
				printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
				fprintf(fas, "    %d var   %s ", i, table[i].name);
				fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case procedur:
				printf("    %d proc  %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(fas,"    %d proc  %s ", i, table[i].name);
				fprintf(fas,"lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			}
		}
		printf("\n");
	}

	/* ���������Ϊ�ֺŻ�end */
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* ÿ��������ż��Ͷ������ϲ������ż��ͣ��Ա㲹�� */
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);			/* �����statement������䴦�� */
	gendo(opr, 0, 0);                       /* ���������ù��̣����ص��õ㲢��ջ��ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ�� */
	memset(nxtlev, 0, sizeof(bool)*symnum); /* �ֳ���û�в��ȼ��� */
	testdo(fsys, nxtlev, 8);                /* �����������ȷ�� */
	listcode(cx0);                          /* ������� */
	return 0;
}

/*
* �����ֱ��м���һ��
*
* k:      ��������const,var or procedure
* ptx:    ���ֱ�βָ���ָ�룬Ϊ�˿��Ըı����ֱ�βָ���ֵ
* lev:    �������ڵĲ��,���Ժ����е�lev��������
* pdx:    dxΪ��ǰӦ����ı�������Ե�ַ�������Ҫ����1
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* ȫ�ֱ���id���Ѵ��е�ǰ���ֵ����� */
	table[(*ptx)].kind = k;
	switch (k)
	{
		case constant:  /* �������� */
			if (num > amax)
			{
				error(31);  /* ��Խ�� */
				num = 0;
			}
			table[(*ptx)].val = num;
			break;
		case variable:  /* �������� */
			table[(*ptx)].level = lev;
			table[(*ptx)].adr = (*pdx);
			(*pdx)++;
			break;
		case procedur:  /*���������֡�*/
			table[(*ptx)].level = lev;
			break;
	}
}

/*
* �������ֵ�λ��.
* �ҵ��򷵻������ֱ��е�λ��,���򷵻�0.
*
* idt:    Ҫ���ҵ�����
* tx:     ��ǰ���ֱ�βָ��
*/
int position(char* idt, int tx)
{
	int i;
	strcpy(table[0].name, idt);
	i = tx;
	while (strcmp(table[i].name, idt) != 0)
	{
		i--;
	}
	return i;
}

/*
* ������������
*/
int constdeclaration(int* ptx, int lev, int* pdx)
{
	if (sym == ident)
	{
		getsymdo;
		if (sym==eql || sym==becomes)
		{
			if (sym == becomes)
			{
				error(1);   /* ��=д����:= */
			}
			getsymdo;
			if (sym == number)
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);   /* ����˵��=��Ӧ������ */
			}
		}
		else
		{
			error(3);   /* ����˵����ʶ��Ӧ��= */
		}
	}
	else
	{
		error(4);   /* const��Ӧ�Ǳ�ʶ */
	}
	return 0;
}

/*
* ������������
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	if (sym == ident)	/* ���ȼ���ǲ��Ǳ�ʶ�� */
	{
		enter(variable, ptx, lev, pdx); /* ��д���ֱ� */
		getsymdo;
	}
	else			
	{
		error(4);   /* var��Ӧ�Ǳ�ʶ */
	}
	return 0;
}

/*
* ���Ŀ������嵥
*/
void listcode(int cx0)
{
	int i;
	if (listswitch)
	{
		for (i=cx0; i<cx; i++)
		{
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			//fprintf(fa,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

void listallcode()
{
	int i;
	//if (listswitch)
	{
		for (i=0; i<cx; i++)
		{
			//printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fa,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*
* ��䴦��
*/
int statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == ident)   /* ׼�����ո�ֵ��䴦�� */
	{
		i = position(id, *ptx);
		if (i == 0)
		{
			error(11);  /* ����δ�ҵ� */
		}
		else
		{
			if(table[i].kind != variable)
			{
				error(12);  /* ��ֵ����ʽ���� */
				i = 0;
			}
			else
			{
				getsymdo;
				if(sym == becomes)
				{
					getsymdo;
				}
				else
				{
					error(13);  /* û�м�⵽��ֵ���� */
				}
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev); /* ����ֵ�����Ҳ���ʽ */
				if(i != 0)
				{
					/* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
					gendo(sto, lev-table[i].level, table[i].adr);
				}
			}
		}
	}
	else
	{
		if (sym == readsym) /* ׼������read��䴦�� */
		{
			getsymdo;
			if (sym != lparen)
			{
				error(34);  /* ��ʽ����Ӧ�������� */
			}
			else
			{
				do {
					getsymdo;
					if (sym == ident)
					{
						i = position(id, *ptx); /* ����Ҫ���ı��� */
					}
					else
					{
						i=0;
					}

					if (i == 0)
					{
						error(35);  /* read()��Ӧ���������ı����� */
					}
					else if (table[i].kind != variable)
					{
						error(32);	/* read()������ı�ʶ�����Ǳ���, thanks to amd */
					}
					else
					{
						gendo(opr, 0, 16);  /* ��������ָ���ȡֵ��ջ�� */
						gendo(sto, lev-table[i].level, table[i].adr);   /* ���浽���� */
					}
					getsymdo;

				} while (sym == comma); /* һ��read���ɶ�������� */
			}
			if(sym != rparen)
			{
				error(33);  /* ��ʽ����Ӧ�������� */
				while (!inset(sym, fsys))   /* �����ȣ�ֱ���յ��ϲ㺯���ĺ������ */
				{
					getsymdo;
				}
			}
			else
			{
				getsymdo;
			}
		}
		else
		{
			if (sym == writesym)    /* ׼������write��䴦����read���� */
			{
				getsymdo;
				if (sym == lparen)
				{
					do {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[rparen] = true;
						nxtlev[comma] = true;       /* write�ĺ������Ϊ) or , */
						expressiondo(nxtlev, ptx, lev); /* ���ñ��ʽ�����˴���read��ͬ��readΪ��������ֵ */
						gendo(opr, 0, 14);  /* �������ָ����ջ����ֵ */
					} while (sym == comma);
					if (sym != rparen)
					{
						error(33);  /* write()��ӦΪ�������ʽ */
					}
					else
					{
						getsymdo;
					}
				}
				gendo(opr, 0, 15);  /* ������� */
			}
			else
			{
				if (sym == callsym) /* ׼������call��䴦�� */
				{
					getsymdo;
					if (sym != ident)
					{
						error(14);  /* call��ӦΪ��ʶ�� */
					}
					else
					{
						i = position(id, *ptx);
						if (i == 0)
						{
							error(11);  /* ����δ�ҵ� */
						}
						else
						{
							if (table[i].kind == procedur)
							{
								getsymdo;
								param_num = 0;	/* ��¼�������� */
								if(sym == lparen){
									do {
										getsymdo;
										memcpy(nxtlev, fsys, sizeof(bool)*symnum);
										expressiondo(nxtlev,ptx,lev);
										param_num++;
									}while(sym == comma);

									if(sym == rparen)
									{
										getsymdo;
									}
									else
									{
										error(55);	/* ���̴����﷨���� */
									}
									gendo(mov, 0, param_num);	/* ��ջ��A��Ԫ��������3��λ�ã�Ϊ���̵��������ڳ�λ�ã�t-A */
								}

								if (param_num != table[i].val)
								{
									error(56);		/* ���̵��ò�����һ�� */
								}
                                /* ��cal L A ���õ�ַΪA�Ĺ��̣�����ڱ���飬���ù����뱻���ù��̲��ΪL
                                �����table[i].adr������code�����λ��, lev�϶����ڵ���table[i].level */
								gendo(cal, lev-table[i].level, table[i].adr);   /* ����callָ�� */
							}
							else
							{
								error(15);  /* call���ʶ��ӦΪ���� */
							} 
						}
					}
				}
				else
				{
					if (sym == ifsym)   /* ׼������if��䴦�� */
					{
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[thensym] = true;
						nxtlev[dosym] = true;   /* �������Ϊthen��do */
						conditiondo(nxtlev, ptx, lev); /* �������������߼����㣩���� */
						if (sym == thensym)
						{
							getsymdo;
						}
						else
						{
							error(16);  /* ȱ��then */
						}
						cx1 = cx;   /* ���浱ǰ������תָ���ַ */
						gendo(jpc, 0, 0);   /* ����������תָ���ת��ַδ֪����ʱд0 */
						statementdo(fsys, ptx, lev);    /* ����then������ */
						if (sym == elsesym)
						{
							cx2 = cx;
						gendo(jmp, 0, 0);
						code[cx1].a = cx;   //��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ
						
							getsymdo;
							statementdo(fsys, ptx, lev);
							code[cx2].a = cx;	//��ǰ��else�����������λ�ã�thenִ����Ӧ����ת����
						}
						else
						{code[cx1].a = cx;
							//statementdo(fsys, ptx, lev);
						}
					}
					else
					{
						if (sym == beginsym)    /* ׼�����ո�����䴦�� */
						{
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool)*symnum);
							nxtlev[semicolon] = true;
							nxtlev[endsym] = true;  /* �������Ϊ�ֺŻ�end */
							/* ѭ��������䴦������ֱ����һ�����Ų�����俪ʼ���Ż��յ�end */
							statementdo(nxtlev, ptx, lev);

							while (inset(sym, statbegsys) || sym==semicolon)
							{
								if (sym == semicolon)
								{
									getsymdo;
								}
								else
								{
									error(10);  /* ȱ�ٷֺ� */
								}
								statementdo(nxtlev, ptx, lev);
							}
							if(sym == endsym)
							{
								getsymdo;
							}
							else
							{
								error(17);  /* ȱ��end��ֺ� */
							}
						}
						else
						{
							if (sym == whilesym)    /* ׼������while��䴦�� */
							{
								cx1 = cx;   /* �����ж�����������λ�� */
								getsymdo;
								memcpy(nxtlev, fsys, sizeof(bool)*symnum);
								nxtlev[dosym] = true;   /* �������Ϊdo */
								conditiondo(nxtlev, ptx, lev);  /* ������������ */
								cx2 = cx;   /* ����ѭ����Ľ�������һ��λ�� */
								gendo(jpc, 0, 0);   /* ����������ת��������ѭ���ĵ�ַδ֪ */
								if (sym == dosym)
								{
									getsymdo;
								}
								else
								{
									error(18);  /* ȱ��do */
								}
								statementdo(fsys, ptx, lev);    /* ѭ���� */
								gendo(jmp, 0, cx1); /* ��ͷ�����ж����� */
								code[cx2].a = cx;   /* ��������ѭ���ĵ�ַ����if���� */
							}
							else
							{
								memset(nxtlev, 0, sizeof(bool)*symnum); /* �������޲��ȼ��� */
								testdo(fsys, nxtlev, 19);   /* �������������ȷ�� */
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* ���ʽ����
*/
int expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;  /* ���ڱ��������� */
	bool nxtlev[symnum];

	if(sym==plus || sym==minus) /* ��ͷ�������ţ���ʱ��ǰ���ʽ������һ�����Ļ򸺵��� */
	{
		addop = sym;    /* ���濪ͷ�������� */
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* ������ */
		if (addop == minus)
		{
			gendo(opr,0,1); /* �����ͷΪ��������ȡ��ָ�� */
		}
	}
	else    /* ��ʱ���ʽ��������ļӼ� */
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* ������ */
	}
	while (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* ������ */
		if (addop == plus)
		{
			gendo(opr, 0, 2);   /* ���ɼӷ�ָ�opr 0 2 ��ջ��+ջ��������Ŵ�ջ����top-1 */
		}
		else
		{
			gendo(opr, 0, 3);   /* ���ɼ���ָ�opr 0 3 ��ջ��-ջ��������Ŵ�ջ����top-1 */
		}
	}
	return 0;
}

/*
* ���
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;  /* ���ڱ���˳������� */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factordo(nxtlev, ptx, lev); /* �������� */
	while(sym==times || sym==slash)
	{
		mulop = sym;
		getsymdo;
		factordo(nxtlev, ptx, lev);
		if(mulop == times)
		{
			gendo(opr, 0, 4);   /* ���ɳ˷�ָ���ջ����ջ����������ջ����top-1 */
		}
		else
		{
			gendo(opr, 0, 5);   /* ���ɳ���ָ���ջ����ջ����������ջ����top-1 */
		}
	}
	return 0;
}

/*
* ���Ӵ���
*/
int factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	testdo(facbegsys, fsys, 24);    /* ������ӵĿ�ʼ���� */
	/* while(inset(sym, facbegsys)) */  /* ѭ��ֱ���������ӿ�ʼ���� */
	if(inset(sym,facbegsys))    /* BUG: ԭ���ķ���var1(var2+var3)�ᱻ����ʶ��Ϊ���� */
	{
		if(sym == ident)    /* ����Ϊ��������� */
		{
			i = position(id, *ptx); /* �������� */
			if (i == 0)
			{
				error(11);  /* ��ʶ��δ���� */
			}
			else
			{
				switch (table[i].kind)
				{
				case constant:  /* ����Ϊ���� */
					gendo(lit, 0, table[i].val);    /* ֱ�Ӱѳ�����ֵ��ջ */
					break;
				case variable:  /* ����Ϊ���� */
					gendo(lod, lev-table[i].level, table[i].adr);   /* �ҵ�������ַ������ֵ��ջ��ȡ��Ե�ǰ���̲��ΪL 
																	����ƫ����ΪA �Ĵ洢��Ԫ��ֵ��ջ��, top+1 */
					break;
				case procedur:  /* ����Ϊ���� */
					error(21);  /* ����Ϊ���� */
					break;
				}
			}
			getsymdo;
		}
		else
		{
			if(sym == number)   /* ����Ϊ�� */
			{
				if (num > amax)
				{
					error(31);
					num = 0;
				}
				gendo(lit, 0, num);
				getsymdo;
			}
			else
			{
				if (sym == lparen)  /* ����Ϊ���ʽ */
				{
					getsymdo;
					memcpy(nxtlev, fsys, sizeof(bool)*symnum);
					nxtlev[rparen] = true;
					expressiondo(nxtlev, ptx, lev);
					if (sym == rparen)
					{
						getsymdo;
					}
					else
					{
						error(22);  /* ȱ�������� */
					}
				}
				testdo(fsys, facbegsys, 23);    /* ���Ӻ��зǷ����� */
			}
		}
	}
	return 0;
}

/*
* ��������
*/
int condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	if(sym == oddsym)   /* ׼������odd���㴦�� */
	{
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);   /* ����oddָ�ջ����ֵ�������������Ϊ1��ż�����Ϊ0 */
	}
	else
	{
		/* �߼����ʽ���� */
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[eql] = true;
		nxtlev[neq] = true;
		nxtlev[lss] = true;
		nxtlev[leq] = true;
		nxtlev[gtr] = true;
		nxtlev[geq] = true;
		expressiondo(nxtlev, ptx, lev);
		if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq)
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsymdo;
			expressiondo(fsys, ptx, lev);
			switch (relop)	/* ��Ԫ�Ƚ�ָ�� */
			{
			case eql:
				gendo(opr, 0, 8);	/* ����ô�ջ��Ϊ1, top-1 */
				break;
			case neq:
				gendo(opr, 0, 9);	/* ������ô�ջ��Ϊ1, top-1 */
				break;
			case lss:
				gendo(opr, 0, 10);	/* С�����ô�ջ��Ϊ1, top-1 */
				break;
			case geq:
				gendo(opr, 0, 11);	/* ���ڵ������ô�ջ��Ϊ1, top-1 */
				break;
			case gtr:
				gendo(opr, 0, 12);	/* �������ô�ջ��Ϊ1, top-1 */
				break;
			case leq:
				gendo(opr, 0, 13);	/* ���ڵ������ô�ջ��Ϊ1, top-1 */
				break;
			}
		}
	}
	return 0;
}

/*
* ���ͳ���
*/
void interpret()
{
	int j;			/* ����ѭ������ */
	int p, b, t;    /* ָ��ָ�룬ָ���ַ��ջ��ָ�� */
	struct instruction i;   /* ��ŵ�ǰָ�� */
	int s[stacksize];   /* ջ */

	printf("start pl0\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	do {
		i = code[p];    /* ����ǰָ�� */
		p++;
		switch (i.f)
		{
		case lit:   /* ��a��ֵȡ��ջ�� */
			s[t] = i.a;
			t++;
			break;
		case opr:   /* ��ѧ���߼����� */
			switch (i.a)
			{
			case 0:		/* ���������ù��̣����ص��õ㲢��ջ */
				t = b;
				p = s[t+2];
				b = s[t+1];
				break;
			case 1:		/* ��ջ��Ԫ���෴������������ջ�� */
				s[t-1] = -s[t-1];
				break;
			case 2:		/* ��ջ����ջ����ӣ������ջ����t-1 */
				t--;
				s[t-1] = s[t-1]+s[t];
				break;
			case 3:		/* ��ջ����ȥջ���������ջ����t-1 */		
				t--;
				s[t-1] = s[t-1]-s[t];
				break;
			case 4:		/* ��ջ�� �� ջ���������ջ����t-1 */
				t--;
				s[t-1] = s[t-1]*s[t];
				break;
			case 5:		/* ��ջ�� ���� ջ���������ջ����t-1 */
				t--;
				s[t-1] = s[t-1]/s[t];
				break;
			case 6:		/* ջ��������Ϊ���� ���Ϊ1��Ϊż����Ϊ0 */
				s[t-1] = s[t-1]%2;
				break;
			case 8:		/* ��ջ����ջ��������ȣ�1���ڴ�ջ����t-1 */
				t--;
				s[t-1] = (s[t-1] == s[t]);
				break;
			case 9:		/* ��ջ����ջ�����ݲ���ȣ�1���ڴ�ջ����t-1 */
				t--;
				s[t-1] = (s[t-1] != s[t]);
				break;
			case 10:	/* ��ջ������С��ջ�����ݣ�1���ڴ�ջ����t-1 */
				t--;
				s[t-1] = (s[t-1] < s[t]);
				break;
			case 11:	/* ��ջ������ �� ջ�����ݣ�1���ڴ�ջ����t-1 */
				t--;
				s[t-1] = (s[t-1] >= s[t]);
				break;
			case 12:	/* ��ջ������ ���� ջ�����ݣ�1���ڴ�ջ����t-1 */
				t--;
				s[t-1] = (s[t-1] > s[t]);
				break;
			case 13:	/* ��ջ������ �� ջ�����ݣ�1���ڴ�ջ����t-1 */
				t--;
				s[t-1] = (s[t-1] <= s[t]);
				break;
			case 14:	/* ջ���������̨��t-1 */
				printf("%d", s[t-1]);
				fprintf(fa2, "%d", s[t-1]);
				t--;
				break;
			case 15:	/* ����̨��Ļ���һ������ */
				printf("\n");
				fprintf(fa2,"\n");
				break;
			case 16:
				printf("?");
				fprintf(fa2, "?");
				scanf("%d", &(s[t]));
				fprintf(fa2, "%d\n", s[t]);
				t++;
				break;
			}
			break;
		case lod:   /* ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ�� */
			s[t] = s[base(i.l,s,b)+i.a];
			t++;
			break;
		case sto:   /* ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ� */
			t--;
			s[base(i.l, s, b) + i.a] = s[t];
			break;
		case cal:   /* �����ӹ��� */
			s[t] = base(i.l, s, b); /* �������̻���ַ��ջ */
			s[t+1] = b; /* �������̻���ַ��ջ������������base���� */
			s[t+2] = p; /* ����ǰָ��ָ����ջ */
			b = t;  /* �ı����ַָ��ֵΪ�¹��̵Ļ���ַ */
			p = i.a;    /* ��ת */
			break;
		case inte:  /* �����ڴ� */
			t += i.a;
			break;
		case jmp:   /* ֱ����ת */
			p = i.a;
			break;
		case jpc:   /* ������ת */
			t--;
			if (s[t] == 0)
			{
				p = i.a;
			}
			break;
		case mov:	/* ��ջ��A��Ԫ��������3��λ��*/
            for(j=i.a; j>0; j--){
                s[t+2] = s[t-1];
                t--;
            }
            break;
		}
	} while (p != 0);
}

/* ͨ�����̻�ַ����l����̵Ļ�ַ */
int base(int l, int* s, int b)
{
	int b1;
	b1 = b;
	while (l > 0)
	{
		b1 = s[b1];
		l--;
	}
	return b1;
}

