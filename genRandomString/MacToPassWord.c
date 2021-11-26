
 #include <string.h>
 #include <memory>
 #include <cctype>
 #include <tchar.h>  // _TCHAR* argv[]
 #include <wtypes.h>
 #include <fstream>
 using namespace std;

char sp[9];
void skimrand()
 {
         for (int s=0;s<100;s++)
                 rand();
         cout<<endl<<"skim 100 rand nums..."<<endl;

}
 void genTo(int count, int sn )
 {        ofstream fout("key.txt",ios::app);

        sn=sn/4;
         for (int k=0;k<count;++k)
         {
                for (int m1=0;m1<sn;++m1)
                 {int temp=(rand()%26)+65;
                  cout<<(char)temp;        fout<<(char)temp;
                }
                 cout<<"-";        fout<<"-";
                 for (int m1=0;m1<sn;++m1)
                 {int temp=rand()%10;
                  cout<<temp;        fout<<temp;
                }
                 cout<<"-";        fout<<"-";
                 for (int m1=0;m1<sn;++m1)
                 {int temp=(rand()%26)+97;
                 cout<<(char)temp;        fout<<(char)temp;
                }
                 cout<<"-";        fout<<"-";
                 for (int m1=0;m1<sn;++m1)
                 {        sp[0]=(char)(rand()%26 +65);        sp[1]=(char)(rand()%26 +97);
                         int temp=rand()%9;
                         cout<<sp[temp];                fout<<sp[temp];
                }
                 cout<<endl;        fout<<endl;

        }
         fout.close();
 }

int _tmain(int argc, _TCHAR* argv[])
 {
        char tcmd;

        sp[2]='@'; sp[3]='#'; sp[4]='$'; sp[5]='%'; sp[6]='^'; sp[7]='&'; sp[8]='*';



        while (cin>>tcmd)
         {
                 switch (tcmd)
                 {
                         case 'g': genTo(10,12); break;
                         case 't': genTo(20,12); break;
                         case 'p': genTo(50,12); break;

                        case 'G': genTo(10,16); break;
                         case 'T': genTo(20,16); break;
                         case 'P': genTo(50,16); break;

                        case 'q': return 0;
                         case 's' : skimrand(); break;
                         default: skimrand(); break;

                }
         }

        return 0;
 }
