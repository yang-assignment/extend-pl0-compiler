/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件?
 * 回答是否输出虚拟机代码
 * 回答是否输出名字表
 * fa.tmp输出虚拟机代码
 * fa1.tmp输出源文件及其各行对应的首地址
 * fa2.tmp输出结?
 * fas.tmp输出名字表
 */

#include <stdio.h>

#include "pl0.h"
#include "string.h"

/* 解释执行时使用的栈 */
#define stacksize 500

int main()
{
	bool nxtlev[symnum];	/* symnum为32, 代表32类符号 */

	printf("Input pl/0 file?   ");
	scanf("%s", fname);     /* 输入文件名 */

	fin = fopen(fname, "r");

	if (fin)	/* 如果这文件存在则Ok */
	{
		//printf("List object code?(Y/N)");   /* 是否输出虚拟机代码 */
		//scanf("%s", fname);
		listswitch = 0; //(fname[0]=='y' || fname[0]=='Y');

		//printf("List symbol table?(Y/N)");  /* 是否输出名字表 */
		//scanf("%s", fname);
		tableswitch = 1; //(fname[0]=='y' || fname[0]=='Y');

		fa1 = fopen("fa1.tmp", "w");
		fprintf(fa1,"Input pl/0 file?   ");
		fprintf(fa1,"%s\n",fname);

		init();     /* 初始化 */

		err = 0; 			/* 计算已发现的错误个数 */
		cc = cx = ll = 0;	/* cc、ll: getch使用的计数器, cc表示当前字符 */
		ch = ' ';			/* 缓冲区中获取字符，getch 使用, 初始化为一个空格 */

		if(-1 != getsym())
		{
			fa = fopen("fa.tmp", "w"); fas = fopen("fas.tmp", "w");
			/* nxtlev：长度为32的bool数组, declbegsys：代表一个声明开始的符号集合，statbegsys代表语句开始的符号集合
               最终nxtlev里就存了3种声明和6种语句的开始符号, 是其中一种其结果就是true */
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;			/* 再加一个程序结束符 . */

			/* 4个参数, 1: 当前分程序所在层, 2: 名字表当前尾指针
            3: 第三个就是标识了3种声明和6种语句的bool数组
			4: param_num参数个数  */
			if(-1 == block(0, 0, nxtlev, 0))   /* 调用编译程序 */
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

			if (sym != period)	/* 如果最后一个符号不是period, 就表示没有正确结束 */
			{
				error(9);
			}

			if (err == 0)		/* 如果只有0个错误, 就可以调用解释程序开始运行了 */
			{
				fa2 = fopen("fa2.tmp", "w");
				listallcode();
				fclose(fa);
				interpret();    /* 调用解释执行程序 */
				fclose(fa2);
			}
			else				/* 否则报错, 无法用解释程序来运行 */
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
* 初始化
*/
void init()
{
	int i;

	/* 设置单字符符号 */
	for (i=0; i<=255; i++)
	{
		ssym[i] = nul;		/* nul = 0 */
	}
	/* ASCII范围(0–31控制字符, 32–126 分配给了能在键盘上找到的字符，数字127代表 DELETE 命令, 后128个是扩展ASCII打印字符) 因此共256个
    此处设置14个单字符的符号值 以ASCII码为索引 构成索引表 */
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

	/* 设置保留字名字,按照字母顺序，便于折半查找，所以else只能放在4号位 */
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
    strcpy(&(word[4][0]), "else");	/* 增加保留字else */
    strcpy(&(word[5][0]), "end");
    strcpy(&(word[6][0]), "if");
    strcpy(&(word[7][0]), "odd");
    strcpy(&(word[8][0]), "procedure");
    strcpy(&(word[9][0]), "read");
    strcpy(&(word[10][0]), "then");
    strcpy(&(word[11][0]), "var");
    strcpy(&(word[12][0]), "while");
    strcpy(&(word[13][0]), "write");

	/* 设置保留字符号 */
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
	/* 设置指令名称 */
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "int");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");
	strcpy(&(mnemonic[mov][0]), "mov");

	/* 设置符号集 */
	for (i=0; i<symnum; i++)	/* symnum = 32，代表32个符号(除去类P_CODE指令) */
	{
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/* 设置声明开始符号集 */
	declbegsys[constsym] = true;	/* 常数声明 */
	declbegsys[varsym] = true;		/* 变量声明 */
	declbegsys[procsym] = true;		/* 过程声明 */

	/* 设置语句开始符号集 */
	statbegsys[beginsym] = true;	/* 复合语句的开始 */
	statbegsys[callsym] = true;		/* 过程调用语句 */
	statbegsys[ifsym] = true;		/* if条件语句 */
	statbegsys[whilesym] = true;	/* while循环语句 */

	/* 设置因子（即等式右边的部分）开始符号集 */
	facbegsys[ident] = true;		/* 标识符 */
	facbegsys[number] = true;		/* 无符号整数 */
	facbegsys[lparen] = true;		/* 左括号 */
}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s)
{
	return s[e];
}

/* sr: 长度为32的bool数组, s1: 代表一个声明开始的符号集合，
s2: 代表语句开始的符号集合, n = 32
最终sr里就存了3种声明和6种语句的开始符号, 是其中一种就是true */
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
*   出错处理，打印出错位置和错误编码
*/
void error(int n)
{
	char space[81];			/* 初始化space, 用于表示错误在一个Line里出现的位置 */
	memset(space,32,81);	/* 32对应的字符为空格 */
	/* 出错时当前符号已经读完，所以cc-1，赋值为0表示'\0'结束, 因此空格的长度就是出错的位置 */
	space[cc-1]=0; 			
	printf("****%s!%d\n", space, n);
	fprintf(fa1,"****%s!%d\n", space, n);
	switch(n)
	{
		case 1 :printf("常量说明中不可以用\":=\". \n");break;
		case 2 :printf("常量说明中的\"=\"后应该是数字.  \n");break;
		case 3 :printf("缺少\"=\". \n");break;
		case 4 :printf("const ，var ，procedure后应为标识符. \n");break;
		case 5 :printf("漏掉了\",\"或者是\";\". \n");break;
		case 6 :printf("过程说明后的符号不正确(应是句子的开始符,过程定义符)。\n");break;
		case 7 :printf("声明顺序有误，应为[<变量说明部分>][<常量说明部分>] [<过程说明部分>]<语句>。\n");break;
		case 8 :printf("程序体内的语句部分的符不正确。\n");break;
		case 9 :printf("程序的末尾丢掉了句号\".\"。\n");break;
		case 10 :printf("句子之间漏掉了\";\"。\n");break;
		case 11 :printf("标识符未声明.\n");break;
		case 12 :printf("赋值号左端应为变量。\n");break;
		case 13 :printf("缺少赋值号 \":=\" 。\n");break;
		case 14 :printf("call 后应为标识符。\n");break;
		case 15 :printf("call 后标识符属性应是过程。\n");break;
		case 16 :printf("缺少\"then\"。\n");break;
		case 17 :printf("缺少\"end\"或\";\"。\n");break;
		case 18 :printf("do while 型循环语句缺少do。\n");break;
		case 19 :printf("语句后的标号不正确。\n");break;
		case 20 :printf("应为关系运算符。\n");break;
		case 21 :printf("表达式内的标识符属性不能是无返回值的过程。\n");break;
		case 22 :printf("表达式中漏掉右括号。\n");break;
		case 23 :printf("表达式后出现非法符号。\n");break;
		case 24 :printf("表达式的开始符为非法符号符号。\n");break;
		case 30 :printf("超出数字最大位数。\n");break;
		case 31 :printf("超过地址上界。\n");break;
		case 32 :printf("超过最大允许过程嵌套声明层数。\n");break;
		case 33 :printf("缺少右括号。");break;
		case 34 :printf("read后缺少左括号 (\n");break;
		case 35 :printf("read里没标识符，或标识符未声明，或者标识符不是变量\n");break;
        default :printf("找不到这种错误\n");
	}

	err++;
}

/*
* 漏掉空格，读取一个字符。
*
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
*
* 被函数getsym调用。
*/
int getch()
{
	/* ll、cc 均初始化为0, cc表示当前字符(ch)的位置
	如果相等了, 说明上次从流中读出的一行字符(在line里)已经被读完了, 就得再读一行出来  */
	if (cc == ll)
	{
		if (feof(fin))
		{
			printf("program incomplete");
			return -1;
		}
		cc = ll = 0;			/* 重新赋值为0 */
		printf("%d ", cx);		/* cx, 虚拟机代码指针，表示这是第几行虚拟机代码 */
		fprintf(fa1,"%d ", cx);
		ch = ' ';				/* ch重置为32，便于进入接下来的循环 */
		while (ch != 10)		/* 刚开始32肯定不等于10, 则进入循环, 10是换行符(意思是把整行读完) */
		{
			if (EOF == fscanf(fin,"%c", &ch))	/* 如果是EOF文件结束符(相当于回车), 则line中该位置为'\0'收尾，并跳出循环 */
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
* 词法分析，获取一个符号
*/
int getsym()
{
	int i,j,k;

	while (ch==' ' || ch==10 || ch==13 || ch==9)  /* 忽略空格、换行、回车和TAB */
	{
		getchdo;			/* 抓取一个字符到ch */
	}
	/* 1. 名字或保留字以a..z开头 */
	if (ch>='a' && ch<='z')	
	{           
		k = 0;				/* 记录当前这个词已经有多少个字符了 */
		do {
			if(k<al)		/* al: 符号的最大长度 */
			{
				a[k] = ch;	/* a: 用于临时读取的一个词(符号) */
				k++;
			}
			getchdo;		/* else：如果后面还有字母数字, k却已经>=al, 则读来丢 */
		} while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
		a[k] = 0;			/* 最后以'\0'结尾 */
		strcpy(id, a);		/* 暂时copy到id中, id: 存放当前标识符 */
		i = 0;
		j = norw-1;			/* norw: 关键字个数, 14个 */
		do {     			/* 搜索当前符号是否为保留字, 折半查找 */
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
		if (i-1 > j)		/* 如果i比j大至少2, 说明是保留字 */
		{
			sym = wsym[k];	/* 通过索引表返回其类型 */
		}
		else				/* 搜索失败则，是名字 */
		{
			sym = ident;	/* sym为标识符类型，其名字还存在id中 */
		}
	}
	/* 2. 不是字母开头, 则只能是数字或符号 */
	else					
	{
		if (ch>='0' && ch<='9')	/* 检测是否为数字：以0..9开头 */
		{           
			k = 0;				/* 表示这个数字已有几位数 */
			num = 0;			/* 记录该数字的值 */
			sym = number;		/* 数字的标识符为number */
			do {
				num = 10*num + ch - '0';	/* 通过累加计算str2num */
				k++;
				getchdo;					/* 再读一个, 不是数字则退出循环 */
			} while (ch>='0' && ch<='9');	/* 获取数字字符 */
			//k--;							/* 读到一个数字才+1，这里不用再减去 */
			if (k > nmax)		/* nmax: number的最大位数, 为14 */
			{
				error(30);		/* 错误编码为30, 数字过长!!! */
			}
		}
	/* 3. 不是字母也不是数字，就是符号(可能是单字符, 也可能是双字符) */
		else
		{
			/* 3.1 检测赋值符号 */
			if (ch == ':')      	
			{
				getchdo;
				if (ch == '=')		/* 如果新读取的是=, 说明整个是 := 赋值 */
				{
					sym = becomes;	/* 赋值符号的标识 */
					getchdo;
				}
				else
				{
					sym = nul;  	/* 没有定义冒号的标识符存在，此处为不能识别的符号 */
				}
			}
			else
			{
			/* 3.2 检测小于或小于等于符号 */
				if (ch == '<')		
				{
					getchdo;
					if (ch == '=')
					{
						sym = leq;	/* 小于等于符号的标识 */
						getchdo;
					}
					else
					{
						sym = lss;	/* 小于符号的标识 */
					}
				}
			/* 3.3 检测大于或大于等于符号 */
				else
				{
					if (ch=='>')	
					{
						getchdo;
						if (ch == '=')
						{
							sym = geq;	/* 大于等于符号的标识 */
							getchdo;
						}
						else
						{
							sym = gtr;	/* 大于符号的标识 */
						}
					}
			/* 3.4 剩下的只能是单字符 */
					else
					{
						/* 如果初始化时有该单字符, 则赋值对应标识, 否则为初始化时的nul标识 */
						sym = ssym[ch];     /* 当符号不满足上述条件时，全部按照单字符符号处理 */
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
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
	if (cx >= cxmax)
	{
		printf("Program too long"); /* 程序过长 */
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}


/*
* 测试当前符号是否合法
*
* 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集?
* （该部分的后跟符号），test负责这项检测，并且负责当检测不通过时的补救措施，
* 程序在需要检测时指定当前需要的符号集合和补救用的集合（如之前未完成部分的后跟
* 符号），以及检测不通过时的错误号。
*
* s1:   我们需要的符号
* s2:   如果不是我们需要的，则需要一个补救用的集?
* n:    错误号
*/
int test(bool* s1, bool* s2, int n)
{
	if (!inset(sym, s1))
	{
		error(n);
		/* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合
           就是排除所有不属于FIRST和FOLLOW的符号 */
		while ((!inset(sym,s1)) && (!inset(sym,s2)))
		{	
			/* 如果是FOLLOW则该句当作结束，getsymdo读取下一个，开始新的分析
        	如果是FIRST，则不用再读，直接开始新一轮的分析 */
			getsymdo;
		}
	}
	return 0;
}

/*
* 编译程序主?
*
* lev:    当前分程序所在层
* tx:     名字表当前尾指针
* fsys:   从main进来的block是：3种声明 + 6种语句的开始符号 + .结束符（是当前模块的FIRST集合）
* ParamNum: 传参参数个数 
*/
int block(int lev, int tx, bool* fsys, int ParamNum)
{
	int i;

	int dx;          		/* 名字分配到的相对地址（相对于该程序，因此每一个proc都会定义一个）*/
	int tx0;        		/* 保留初始tx：名字表中最后一个名字的指针 */
	int cx0;        		/* 保留初始cx */
	bool nxtlev[symnum];	/* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空?
							传递给下级函数*/

	dx = 3 + ParamNum;				/* 从下到上依次：动态链（调用者地址）、静态链（父亲地址）、返回值（断点） */
	tx0 = tx;               			/* 记录本层名字的初始位置 */
	table[tx0-ParamNum].adr = cx;	/* table这个位置不会存放其他名字，就是main或者procedure，
										先用于暂时存放jmp在code里的位置cx，等声明分析后语句执行前修改；
										然后再改成inte的位置 */
	gendo(jmp, 0, 0);					/* 首先就生成无条件跳转jmp 0 0, 为了跳到主函数虚拟机代码处，后面会改 */

	if (lev > levmax)
	{
		error(32);
	}

	do 
	{
		if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
		{
			getsymdo;

			constdeclarationdo(&tx, lev, &dx);  /* dx的值会被constdeclaration改变，使用指针 */
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
				error(5);   /*漏掉了逗号或者分号*/
			}
		}

		if (sym == varsym)      /* 收到变量声明符号，开始处理变量声明 */
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

		while (sym == procsym) /* 收到过程声明符号，开始处理过程声明 */
		{
			getsymdo;

			if (sym == ident)
			{
				enter(procedur, &tx, lev, &dx); /* 记录过程名字 */
				getsymdo;
			}
			else
			{
				error(4);   	/* procedure后应为标识符 */
			}

			save_dx = dx;		/* 暂存dx */
			dx = 3;				/* dx变成3, 使得传参变量都从过程中dx=3的位置开始记录 */
			int tx_cp = tx;		/* 暂存当前proc的名字表地址，用于回填参数数量 */
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
					error(55);		/* 过程传参语法错误 */
				}

				table[tx_cp].val = param_num;	/* 记录过程参数个数到val */
			}
			dx = save_dx;			/* 恢复原来dx */

			if (sym == semicolon)	/* procedure的 标识符后应该有个 ; */
			{
				getsymdo;
			}
			else
			{
				error(5);   		/* 漏掉了分号 */
			}
			
			memcpy(nxtlev, fsys, sizeof(bool)*symnum);	/* fsys: 当前模块的FIRST集合 */
			nxtlev[semicolon] = true;	/* nxtlev下层的FIRST集合，继承了这一层的 3种声明 + 6种语句的开始符号 + .结束符，外加一个； */
			if (-1 == block(lev+1, tx, nxtlev, param_num))	/* 此处tx是在参数的位置之后的 */
			{
				return -1;  /* 递归调用 */
			}
			tx = tx_cp; 	/* tx也回归本层 */

			if(sym == semicolon)
			{
				/* <分程序> ::= [<常量说明>][<变量说明>][<过程说明>]<语句>  注意这里的<语句>只能有一个
				这里是说接下来可以是7种<语句>，或者是再定义一个procedure */
				getsymdo;
				memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
				nxtlev[ident] = true;	
				nxtlev[procsym] = true;	/* 加上这个的意思是: 还可以继续声明第二个procedure */
				/* 错误处理用的代码，这里是对<分程序>接下来的<语句>部分进行test
				fsys是此层的FIRST：3种声明 + 6种语句的开始符号 + .结束符 */
				testdo(nxtlev, fsys, 6);
			}
			else
			{
				error(5);   /* 漏掉了分号 */
			}
		}
		memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));   /* 直到没有声明符号 */
	/* 这里的table[tx0].adr就是jmp在code里的位置，即修改jmp的a，使之直接跳到程序入口, 即inte代码处 */
	code[table[tx0-ParamNum].adr].a = cx;    	/* 开始生成当前过程代码 */
	/* 这里table[tx0] 其实是程序procedure或主程序在名字表中的实例, adr就直接指向inte在code中的位置, 当然此时cx还指向的空，但立刻这个空就会是inte */
	table[tx0-ParamNum].adr = cx;	/* 当前过程的入口地址 */
	/* size就是dx: 3+声明变量数 */
	table[tx0-ParamNum].size = dx;	/* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
	cx0 = cx;
	gendo(inte, 0, dx);             	/* 生成分配内存代码，dx就是size大小 */

	if (tableswitch)        			/* 输出名字表 */
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

	/* 语句后跟符号为分号或end */
	memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* 每个后跟符号集和都包含上层后跟符号集和，以便补救 */
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);			/* 这里的statement就是语句处理 */
	gendo(opr, 0, 0);                       /* 结束被调用过程，返回调用点并退栈，每个过程出口都要使用的释放数据段指令 */
	memset(nxtlev, 0, sizeof(bool)*symnum); /* 分程序没有补救集合 */
	testdo(fsys, nxtlev, 8);                /* 检测后跟符号正确性 */
	listcode(cx0);                          /* 输出代码 */
	return 0;
}

/*
* 在名字表中加入一项
*
* k:      名字种类const,var or procedure
* ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:    名字所在的层次,，以后所有的lev都是这样
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
	(*ptx)++;
	strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
	table[(*ptx)].kind = k;
	switch (k)
	{
		case constant:  /* 常量名字 */
			if (num > amax)
			{
				error(31);  /* 数越界 */
				num = 0;
			}
			table[(*ptx)].val = num;
			break;
		case variable:  /* 变量名字 */
			table[(*ptx)].level = lev;
			table[(*ptx)].adr = (*pdx);
			(*pdx)++;
			break;
		case procedur:  /*　过程名字　*/
			table[(*ptx)].level = lev;
			break;
	}
}

/*
* 查找名字的位置.
* 找到则返回在名字表中的位置,否则返回0.
*
* idt:    要查找的名字
* tx:     当前名字表尾指针
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
* 常量声明处理
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
				error(1);   /* 把=写成了:= */
			}
			getsymdo;
			if (sym == number)
			{
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else
			{
				error(2);   /* 常量说明=后应是数字 */
			}
		}
		else
		{
			error(3);   /* 常量说明标识后应是= */
		}
	}
	else
	{
		error(4);   /* const后应是标识 */
	}
	return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	if (sym == ident)	/* 首先检测是不是标识符 */
	{
		enter(variable, ptx, lev, pdx); /* 填写名字表 */
		getsymdo;
	}
	else			
	{
		error(4);   /* var后应是标识 */
	}
	return 0;
}

/*
* 输出目标代码清单
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
* 语句处理
*/
int statement(bool* fsys, int* ptx, int lev)
{
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == ident)   /* 准备按照赋值语句处理 */
	{
		i = position(id, *ptx);
		if (i == 0)
		{
			error(11);  /* 变量未找到 */
		}
		else
		{
			if(table[i].kind != variable)
			{
				error(12);  /* 赋值语句格式错误 */
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
					error(13);  /* 没有检测到赋值符号 */
				}
				memcpy(nxtlev, fsys, sizeof(bool)*symnum);
				expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
				if(i != 0)
				{
					/* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
					gendo(sto, lev-table[i].level, table[i].adr);
				}
			}
		}
	}
	else
	{
		if (sym == readsym) /* 准备按照read语句处理 */
		{
			getsymdo;
			if (sym != lparen)
			{
				error(34);  /* 格式错误，应是左括号 */
			}
			else
			{
				do {
					getsymdo;
					if (sym == ident)
					{
						i = position(id, *ptx); /* 查找要读的变量 */
					}
					else
					{
						i=0;
					}

					if (i == 0)
					{
						error(35);  /* read()中应是声明过的变量名 */
					}
					else if (table[i].kind != variable)
					{
						error(32);	/* read()参数表的标识符不是变量, thanks to amd */
					}
					else
					{
						gendo(opr, 0, 16);  /* 生成输入指令，读取值到栈顶 */
						gendo(sto, lev-table[i].level, table[i].adr);   /* 储存到变量 */
					}
					getsymdo;

				} while (sym == comma); /* 一条read语句可读多个变量 */
			}
			if(sym != rparen)
			{
				error(33);  /* 格式错误，应是右括号 */
				while (!inset(sym, fsys))   /* 出错补救，直到收到上层函数的后跟符号 */
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
			if (sym == writesym)    /* 准备按照write语句处理，与read类似 */
			{
				getsymdo;
				if (sym == lparen)
				{
					do {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[rparen] = true;
						nxtlev[comma] = true;       /* write的后跟符号为) or , */
						expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
						gendo(opr, 0, 14);  /* 生成输出指令，输出栈顶的值 */
					} while (sym == comma);
					if (sym != rparen)
					{
						error(33);  /* write()中应为完整表达式 */
					}
					else
					{
						getsymdo;
					}
				}
				gendo(opr, 0, 15);  /* 输出换行 */
			}
			else
			{
				if (sym == callsym) /* 准备按照call语句处理 */
				{
					getsymdo;
					if (sym != ident)
					{
						error(14);  /* call后应为标识符 */
					}
					else
					{
						i = position(id, *ptx);
						if (i == 0)
						{
							error(11);  /* 过程未找到 */
						}
						else
						{
							if (table[i].kind == procedur)
							{
								getsymdo;
								param_num = 0;	/* 记录参数个数 */
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
										error(55);	/* 过程传参语法错误 */
									}
									gendo(mov, 0, param_num);	/* 将栈顶A个元素往上移3个位置，为过程的链数据腾出位置，t-A */
								}

								if (param_num != table[i].val)
								{
									error(56);		/* 过程调用参数不一致 */
								}
                                /* 用cal L A 调用地址为A的过程，这个在表里查，调用过程与被调用过程层差为L
                                这里的table[i].adr就是在code的相对位置, lev肯定大于等于table[i].level */
								gendo(cal, lev-table[i].level, table[i].adr);   /* 生成call指令 */
							}
							else
							{
								error(15);  /* call后标识符应为过程 */
							} 
						}
					}
				}
				else
				{
					if (sym == ifsym)   /* 准备按照if语句处理 */
					{
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)*symnum);
						nxtlev[thensym] = true;
						nxtlev[dosym] = true;   /* 后跟符号为then或do */
						conditiondo(nxtlev, ptx, lev); /* 调用条件处理（逻辑运算）函数 */
						if (sym == thensym)
						{
							getsymdo;
						}
						else
						{
							error(16);  /* 缺少then */
						}
						cx1 = cx;   /* 保存当前条件跳转指令地址 */
						gendo(jpc, 0, 0);   /* 生成条件跳转指令，跳转地址未知，暂时写0 */
						statementdo(fsys, ptx, lev);    /* 处理then后的语句 */
						if (sym == elsesym)
						{
							cx2 = cx;
						gendo(jmp, 0, 0);
						code[cx1].a = cx;   //经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址
						
							getsymdo;
							statementdo(fsys, ptx, lev);
							code[cx2].a = cx;	//当前是else后面的语句结束位置，then执行完应当跳转至此
						}
						else
						{code[cx1].a = cx;
							//statementdo(fsys, ptx, lev);
						}
					}
					else
					{
						if (sym == beginsym)    /* 准备按照复合语句处理 */
						{
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool)*symnum);
							nxtlev[semicolon] = true;
							nxtlev[endsym] = true;  /* 后跟符号为分号或end */
							/* 循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end */
							statementdo(nxtlev, ptx, lev);

							while (inset(sym, statbegsys) || sym==semicolon)
							{
								if (sym == semicolon)
								{
									getsymdo;
								}
								else
								{
									error(10);  /* 缺少分号 */
								}
								statementdo(nxtlev, ptx, lev);
							}
							if(sym == endsym)
							{
								getsymdo;
							}
							else
							{
								error(17);  /* 缺少end或分号 */
							}
						}
						else
						{
							if (sym == whilesym)    /* 准备按照while语句处理 */
							{
								cx1 = cx;   /* 保存判断条件操作的位置 */
								getsymdo;
								memcpy(nxtlev, fsys, sizeof(bool)*symnum);
								nxtlev[dosym] = true;   /* 后跟符号为do */
								conditiondo(nxtlev, ptx, lev);  /* 调用条件处理 */
								cx2 = cx;   /* 保存循环体的结束的下一个位置 */
								gendo(jpc, 0, 0);   /* 生成条件跳转，但跳出循环的地址未知 */
								if (sym == dosym)
								{
									getsymdo;
								}
								else
								{
									error(18);  /* 缺少do */
								}
								statementdo(fsys, ptx, lev);    /* 循环体 */
								gendo(jmp, 0, cx1); /* 回头重新判断条件 */
								code[cx2].a = cx;   /* 反填跳出循环的地址，与if类似 */
							}
							else
							{
								memset(nxtlev, 0, sizeof(bool)*symnum); /* 语句结束无补救集合 */
								testdo(fsys, nxtlev, 19);   /* 检测语句结束的正确性 */
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
* 表达式处理
*/
int expression(bool* fsys, int* ptx, int lev)
{
	enum symbol addop;  /* 用于保存正负号 */
	bool nxtlev[symnum];

	if(sym==plus || sym==minus) /* 开头的正负号，此时当前表达式被看作一个正的或负的项 */
	{
		addop = sym;    /* 保存开头的正负号 */
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
		if (addop == minus)
		{
			gendo(opr,0,1); /* 如果开头为负号生成取负指令 */
		}
	}
	else    /* 此时表达式被看作项的加减 */
	{
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
	}
	while (sym==plus || sym==minus)
	{
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool)*symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);   /* 处理项 */
		if (addop == plus)
		{
			gendo(opr, 0, 2);   /* 生成加法指令：opr 0 2 次栈顶+栈顶，结果放次栈顶，top-1 */
		}
		else
		{
			gendo(opr, 0, 3);   /* 生成减法指令：opr 0 3 次栈顶-栈顶，结果放次栈顶，top-1 */
		}
	}
	return 0;
}

/*
* 项处理
*/
int term(bool* fsys, int* ptx, int lev)
{
	enum symbol mulop;  /* 用于保存乘除法符号 */
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool)*symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factordo(nxtlev, ptx, lev); /* 处理因子 */
	while(sym==times || sym==slash)
	{
		mulop = sym;
		getsymdo;
		factordo(nxtlev, ptx, lev);
		if(mulop == times)
		{
			gendo(opr, 0, 4);   /* 生成乘法指令：次栈顶×栈顶，结果存次栈顶，top-1 */
		}
		else
		{
			gendo(opr, 0, 5);   /* 生成除法指令：次栈顶÷栈顶，结果存次栈顶，top-1 */
		}
	}
	return 0;
}

/*
* 因子处理
*/
int factor(bool* fsys, int* ptx, int lev)
{
	int i;
	bool nxtlev[symnum];
	testdo(facbegsys, fsys, 24);    /* 检测因子的开始符号 */
	/* while(inset(sym, facbegsys)) */  /* 循环直到不是因子开始符号 */
	if(inset(sym,facbegsys))    /* BUG: 原来的方法var1(var2+var3)会被错误识别为因子 */
	{
		if(sym == ident)    /* 因子为常量或变量 */
		{
			i = position(id, *ptx); /* 查找名字 */
			if (i == 0)
			{
				error(11);  /* 标识符未声明 */
			}
			else
			{
				switch (table[i].kind)
				{
				case constant:  /* 名字为常量 */
					gendo(lit, 0, table[i].val);    /* 直接把常量的值入栈 */
					break;
				case variable:  /* 名字为变量 */
					gendo(lod, lev-table[i].level, table[i].adr);   /* 找到变量地址并将其值入栈：取相对当前过程层差为L 
																	数据偏移量为A 的存储单元的值到栈顶, top+1 */
					break;
				case procedur:  /* 名字为过程 */
					error(21);  /* 不能为过程 */
					break;
				}
			}
			getsymdo;
		}
		else
		{
			if(sym == number)   /* 因子为数 */
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
				if (sym == lparen)  /* 因子为表达式 */
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
						error(22);  /* 缺少右括号 */
					}
				}
				testdo(fsys, facbegsys, 23);    /* 因子后有非法符号 */
			}
		}
	}
	return 0;
}

/*
* 条件处理
*/
int condition(bool* fsys, int* ptx, int lev)
{
	enum symbol relop;
	bool nxtlev[symnum];

	if(sym == oddsym)   /* 准备按照odd运算处理 */
	{
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);   /* 生成odd指令：栈顶的值，若是奇数则变为1，偶数则变为0 */
	}
	else
	{
		/* 逻辑表达式处理 */
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
			switch (relop)	/* 二元比较指令 */
			{
			case eql:
				gendo(opr, 0, 8);	/* 相等让次栈顶为1, top-1 */
				break;
			case neq:
				gendo(opr, 0, 9);	/* 不相等让次栈顶为1, top-1 */
				break;
			case lss:
				gendo(opr, 0, 10);	/* 小于则让次栈顶为1, top-1 */
				break;
			case geq:
				gendo(opr, 0, 11);	/* 大于等于则让次栈顶为1, top-1 */
				break;
			case gtr:
				gendo(opr, 0, 12);	/* 大于则让次栈顶为1, top-1 */
				break;
			case leq:
				gendo(opr, 0, 13);	/* 大于等于则让次栈顶为1, top-1 */
				break;
			}
		}
	}
	return 0;
}

/*
* 解释程序
*/
void interpret()
{
	int j;			/* 用于循环遍历 */
	int p, b, t;    /* 指令指针，指令基址，栈顶指针 */
	struct instruction i;   /* 存放当前指令 */
	int s[stacksize];   /* 栈 */

	printf("start pl0\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	do {
		i = code[p];    /* 读当前指令 */
		p++;
		switch (i.f)
		{
		case lit:   /* 将a的值取到栈顶 */
			s[t] = i.a;
			t++;
			break;
		case opr:   /* 数学、逻辑运算 */
			switch (i.a)
			{
			case 0:		/* 结束被调用过程，返回调用点并退栈 */
				t = b;
				p = s[t+2];
				b = s[t+1];
				break;
			case 1:		/* 求栈顶元素相反数，并保存在栈顶 */
				s[t-1] = -s[t-1];
				break;
			case 2:		/* 次栈顶与栈顶相加，存入次栈顶，t-1 */
				t--;
				s[t-1] = s[t-1]+s[t];
				break;
			case 3:		/* 次栈顶减去栈顶，存入次栈顶，t-1 */		
				t--;
				s[t-1] = s[t-1]-s[t];
				break;
			case 4:		/* 次栈顶 × 栈顶，存入次栈顶，t-1 */
				t--;
				s[t-1] = s[t-1]*s[t];
				break;
			case 5:		/* 次栈顶 除以 栈顶，存入次栈顶，t-1 */
				t--;
				s[t-1] = s[t-1]/s[t];
				break;
			case 6:		/* 栈顶内容若为奇数 则变为1，为偶数变为0 */
				s[t-1] = s[t-1]%2;
				break;
			case 8:		/* 次栈顶与栈顶内容相等，1存于次栈顶，t-1 */
				t--;
				s[t-1] = (s[t-1] == s[t]);
				break;
			case 9:		/* 次栈顶与栈顶内容不相等，1存于次栈顶，t-1 */
				t--;
				s[t-1] = (s[t-1] != s[t]);
				break;
			case 10:	/* 次栈顶内容小于栈顶内容，1存于次栈顶，t-1 */
				t--;
				s[t-1] = (s[t-1] < s[t]);
				break;
			case 11:	/* 次栈顶内容 ≥ 栈顶内容，1存于次栈顶，t-1 */
				t--;
				s[t-1] = (s[t-1] >= s[t]);
				break;
			case 12:	/* 次栈顶内容 大于 栈顶内容，1存于次栈顶，t-1 */
				t--;
				s[t-1] = (s[t-1] > s[t]);
				break;
			case 13:	/* 次栈顶内容 ≤ 栈顶内容，1存于次栈顶，t-1 */
				t--;
				s[t-1] = (s[t-1] <= s[t]);
				break;
			case 14:	/* 栈顶输出控制台，t-1 */
				printf("%d", s[t-1]);
				fprintf(fa2, "%d", s[t-1]);
				t--;
				break;
			case 15:	/* 控制台屏幕输出一个换行 */
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
		case lod:   /* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
			s[t] = s[base(i.l,s,b)+i.a];
			t++;
			break;
		case sto:   /* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
			t--;
			s[base(i.l, s, b) + i.a] = s[t];
			break;
		case cal:   /* 调用子过程 */
			s[t] = base(i.l, s, b); /* 将父过程基地址入栈 */
			s[t+1] = b; /* 将本过程基地址入栈，此两项用于base函数 */
			s[t+2] = p; /* 将当前指令指针入栈 */
			b = t;  /* 改变基地址指针值为新过程的基地址 */
			p = i.a;    /* 跳转 */
			break;
		case inte:  /* 分配内存 */
			t += i.a;
			break;
		case jmp:   /* 直接跳转 */
			p = i.a;
			break;
		case jpc:   /* 条件跳转 */
			t--;
			if (s[t] == 0)
			{
				p = i.a;
			}
			break;
		case mov:	/* 将栈顶A个元素往上移3个位置*/
            for(j=i.a; j>0; j--){
                s[t+2] = s[t-1];
                t--;
            }
            break;
		}
	} while (p != 0);
}

/* 通过过程基址求上l层过程的基址 */
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

