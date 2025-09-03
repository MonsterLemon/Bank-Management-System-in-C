#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#define MAX_ACCOUNTS 5000
#define NAME_MAX_LEN 50
#define PASS_MAX_LEN 32

// Account Types
typedef enum { TYPE_JOBHOLDER=1, TYPE_STUDENT=2, TYPE_WOMEN=3 } AccType;

typedef struct {
    char accNo[13];
    char name[NAME_MAX_LEN+1];
    char password[PASS_MAX_LEN];
    AccType type;
    double balance;
} Account;


static Account accounts[MAX_ACCOUNTS];
static int accountCount = 0;
static int nextSuffix = 1;
static bool isInit = true;


static void clearLine(void){
    int c;
    while((c=getchar())!='\n' && c!=EOF) {}
}


static void safeReadLine(char *b, size_t n){
    if(!b || n==0) return;
    if (fgets(b, (int)n, stdin)) {
        size_t m = strlen(b);

        while (m > 0 && isspace((unsigned char)b[m-1])) {
            b[--m] = '\0';
        }

        size_t i = 0;
        while (b[i] && isspace((unsigned char)b[i])) i++;
        if (i) memmove(b, b+i, strlen(b+i)+1);
    } else {
        b[0] = '\0';
        clearLine();
    }
}

static void promptIntFlush(const char *p){ printf("%s", p); fflush(stdout); }
static int readIntPrompt(const char *p){
    int x;
    for(;;){
        promptIntFlush(p);
        if(scanf("%d",&x)==1){ clearLine(); return x; }
        printf("Invalid.\n"); clearLine();
    }
}
static double readDoublePrompt(const char *p){
    double x;
    for(;;){
        promptIntFlush(p);
        if(scanf("%lf",&x)==1){ clearLine(); return x; }
        printf("Invalid.\n"); clearLine();
    }
}

static const char* typeToString(AccType t){
    return t==1 ? "JOBHOLDER" : t==2 ? "STUDENT" : t==3 ? "WOMEN" : "UNKNOWN";
}
static int minBalanceForType(AccType t){
    return t==1 ? 2000 : t==2 ? 1000 : t==3 ? 1500 : 0;
}
static double pow1p(double r,int n){
    double b = 1 + r, a = 1;
    while(n){ if(n & 1) a *= b; b *= b; n >>= 1; }
    return a;
}

static double now_seconds(void){
    struct timespec ts;
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return ts.tv_sec + ts.tv_nsec/1e9;
    }
#endif
    return (double)clock() / CLOCKS_PER_SEC;
}



static void generateAccountNumber(char out[13]){
    if(nextSuffix > 999999999) snprintf(out, 13, "333999999999");
    else snprintf(out, 13, "333%09d", nextSuffix++);
}

static void saveAll(void){
    FILE *f = fopen("accounts.csv","w");
    if (!f) return;
    fprintf(f,"accNo,name,type,balance,password\n");
    for(int i=0;i<accountCount;i++){
        fprintf(f,"%s,%s,%s,%.2f,%s\n",
                accounts[i].accNo,
                accounts[i].name,
                typeToString(accounts[i].type),
                accounts[i].balance,
                accounts[i].password);
    }
    fclose(f);
    if(!isInit) printf("[SAVE] %d accounts saved\n", accountCount);
}


static int findIndexByAccNo(const char *s){
    for(int i=0;i<accountCount;i++) if(strcmp(accounts[i].accNo, s) == 0) return i;
    return -1;
}
static void displayAccounts(void){
    if(!accountCount){ puts("No accounts."); return; }
    printf("\n%-12s | %-20s | %-10s | %-10s\n", "ACCOUNT NO", "NAME", "TYPE", "BALANCE");
    for(int i=0;i<accountCount;i++){
        printf("%-12s | %-20s | %-10s | %.2f\n",
               accounts[i].accNo, accounts[i].name, typeToString(accounts[i].type), accounts[i].balance);
    }
}


static void merge(Account a[], int l, int m, int r){
    int n1 = m - l + 1, n2 = r - m;
    Account *L = malloc(n1 * sizeof *L);
    Account *R = malloc(n2 * sizeof *R);
    if (!L || !R) { free(L); free(R); return; }
    for(int i=0;i<n1;i++) L[i] = a[l+i];
    for(int j=0;j<n2;j++) R[j] = a[m+1+j];
    int i=0,j=0,k=l;
    while(i<n1 && j<n2){
        if(strcmp(L[i].accNo, R[j].accNo) <= 0) a[k++] = L[i++];
        else a[k++] = R[j++];
    }
    while(i<n1) a[k++] = L[i++];
    while(j<n2) a[k++] = R[j++];
    free(L); free(R);
}
static void mergeSort(Account a[], int l, int r){
    if(l < r){
        int m = l + (r - l) / 2;
        mergeSort(a, l, m);
        mergeSort(a, m+1, r);
        merge(a, l, m, r);
    }
}


static int partition(Account a[], int lo, int hi){
    double p = a[hi].balance; int i = lo - 1;
    for(int j=lo;j<hi;j++){
        if(a[j].balance < p){ ++i; Account t=a[i]; a[i]=a[j]; a[j]=t; }
    }
    Account t = a[i+1]; a[i+1]=a[hi]; a[hi]=t;
    return i+1;
}
static void quickSort(Account a[], int lo, int hi){
    if(lo < hi){
        int p = partition(a, lo, hi);
        quickSort(a, lo, p-1);
        quickSort(a, p+1, hi);
    }
}

static void copyArray(Account src[], Account dest[], int n){
    for(int i=0;i<n;i++) dest[i]=src[i];
}

static void sortByAccNo(void){
    if(accountCount <= 1){ puts("Not enough accounts to sort."); return; }
    Account temp[MAX_ACCOUNTS];
    double t1, t2;

    printf("\n=== MERGE SORT (Sorted by Account Number) ===\n");

    //Best Case
    copyArray(accounts, temp, accountCount);
    mergeSort(temp, 0, accountCount-1); /* pre-sort */
    t1 = now_seconds();
    mergeSort(temp, 0, accountCount-1);
    t2 = now_seconds();
    printf("[Best Case]    Time: %.9f sec\n", t2 - t1);

    // Average Case:
    copyArray(accounts, temp, accountCount);
    for(int i=0;i<accountCount;i++){ int j = rand() % accountCount; Account t = temp[i]; temp[i]=temp[j]; temp[j]=t; }
    t1 = now_seconds();
    mergeSort(temp, 0, accountCount-1);
    t2 = now_seconds();
    printf("[Average Case] Time: %.9f sec\n", t2 - t1);

    // Worst Case:
    copyArray(accounts, temp, accountCount);
    mergeSort(temp, 0, accountCount-1);
    for(int i=0;i<accountCount/2;i++){ Account t = temp[i]; temp[i] = temp[accountCount-1-i]; temp[accountCount-1-i] = t; }
    t1 = now_seconds();
    mergeSort(temp, 0, accountCount-1);
    t2 = now_seconds();
    printf("[Worst Case]    Time: %.9f sec\n", t2 - t1);


    mergeSort(temp, 0, accountCount-1);
    printf("\n--- Final Sorted Accounts (AccNo) ---\n");
    printf("%-12s | %-20s | %-10s | %-10s\n","ACCOUNT NO","NAME","TYPE","BALANCE");
    for(int i=0;i<accountCount;i++){
        printf("%-12s | %-20s | %-10s | %.2f\n",
               temp[i].accNo, temp[i].name, typeToString(temp[i].type), temp[i].balance);
    }
}

static void sortByBalance(void){
    if(accountCount <= 1){ puts("Not enough accounts to sort."); return; }
    Account temp[MAX_ACCOUNTS];
    double t1, t2;

    printf("\n=== QUICK SORT (Sorted by Balance) ===\n");

    // Best Case:
    copyArray(accounts, temp, accountCount);
    quickSort(temp, 0, accountCount-1); /* pre-sort */
    t1 = now_seconds();
    quickSort(temp, 0, accountCount-1);
    t2 = now_seconds();
    printf("[Best Case]    Time: %.9f sec\n", t2 - t1);

    // Average Case:
    copyArray(accounts, temp, accountCount);
    for(int i=0;i<accountCount;i++){ int j = rand() % accountCount; Account t = temp[i]; temp[i]=temp[j]; temp[j]=t; }
    t1 = now_seconds();
    quickSort(temp, 0, accountCount-1);
    t2 = now_seconds();
    printf("[Average Case] Time: %.9f sec\n", t2 - t1);

    //Worst Case:
    copyArray(accounts, temp, accountCount);
    quickSort(temp, 0, accountCount-1);
    for(int i=0;i<accountCount/2;i++){ Account t = temp[i]; temp[i] = temp[accountCount-1-i]; temp[accountCount-1-i] = t; }
    t1 = now_seconds();
    quickSort(temp, 0, accountCount-1);
    t2 = now_seconds();
    printf("[Worst Case]    Time: %.9f sec\n", t2 - t1);


    quickSort(temp, 0, accountCount-1);
    printf("\n--- Final Sorted Accounts (Balance) ---\n");
    printf("%-12s | %-20s | %-10s | %-10s\n","ACCOUNT NO","NAME","TYPE","BALANCE");
    for(int i=0;i<accountCount;i++){
        printf("%-12s | %-20s | %-10s | %.2f\n",
               temp[i].accNo, temp[i].name, typeToString(temp[i].type), temp[i].balance);
    }
}

//CRUD
static AccType readTypeChoice(void){
    for(;;){
        int t = readIntPrompt("Type (1=JobHolder,2=Student,3=WOMEN): ");
        if(t >= 1 && t <= 3) return (AccType)t;
        puts("Invalid.");
    }
}
static void addAccount(void){
    if(accountCount >= MAX_ACCOUNTS){ puts("Bank full."); return; }
    Account a;
    generateAccountNumber(a.accNo);
    printf("Enter Name: "); fflush(stdout);
    safeReadLine(a.name, sizeof a.name);
    a.name[NAME_MAX_LEN] = '\0';
    a.type = readTypeChoice();
    double init = readDoublePrompt("Initial Deposit: ");
    int minB = minBalanceForType(a.type);
    if(init < minB){ printf("Minimum %d required.\n", minB); return; }
    a.balance = init;
    printf("Set Password: "); fflush(stdout);
    safeReadLine(a.password, sizeof a.password);
    if(!strlen(a.password)){ puts("Password empty."); return; }
    accounts[accountCount++] = a;
    saveAll();
    puts("[ADD] Account created.");
}
static void updateAccount(void){
    char s[13];
    printf("Enter AccNo: "); fflush(stdout);
    safeReadLine(s, sizeof s);
    int i = findIndexByAccNo(s); if(i < 0){ puts("Not found."); return; }
    printf("Enter new Name: "); fflush(stdout);
    safeReadLine(accounts[i].name, sizeof accounts[i].name);
    printf("Change Type? (y/n): "); fflush(stdout);
    char c[8]; safeReadLine(c, sizeof c);
    if(c[0] == 'y' || c[0] == 'Y') accounts[i].type = readTypeChoice();
    printf("Change Password? (y/n): "); fflush(stdout);
    safeReadLine(c, sizeof c);
    if(c[0] == 'y' || c[0] == 'Y'){ printf("New pass: "); fflush(stdout); safeReadLine(accounts[i].password, sizeof accounts[i].password); }
    saveAll(); puts("[UPDATE] Done.");
}
static void deleteAccount(void){
    char s[13];
    printf("Enter AccNo: "); fflush(stdout);
    safeReadLine(s, sizeof s);
    int i = findIndexByAccNo(s); if(i < 0){ puts("Not found."); return; }
    for(int k=i;k<accountCount-1;k++) accounts[k] = accounts[k+1];
    accountCount--; saveAll(); puts("[DELETE] Done.");
}


static void depositAtIndex(int i){
    double a = readDoublePrompt("Deposit: ");
    if(a <= 0){ puts("Invalid."); return; }
    accounts[i].balance += a; saveAll(); puts("[DEPOSIT] Done.");
}
static void withdrawAtIndex(int i){
    double a = readDoublePrompt("Withdraw: ");
    if(a <= 0){ puts("Invalid."); return; }
    int m = minBalanceForType(accounts[i].type);
    if(accounts[i].balance - a < m){ puts("Denied."); return; }
    accounts[i].balance -= a; saveAll(); puts("[WITHDRAW] Done.");
}
static void depositByAcc(void){
    char s[13];
    printf("AccNo: "); fflush(stdout);
    safeReadLine(s, sizeof s);
    int i = findIndexByAccNo(s); if(i < 0){ puts("Not found."); return; }
    depositAtIndex(i);
}
static void withdrawByAcc(void){
    char s[13];
    printf("AccNo: "); fflush(stdout);
    safeReadLine(s, sizeof s);
    int i = findIndexByAccNo(s); if(i < 0){ puts("Not found."); return; }
    withdrawAtIndex(i);
}


static void searchAccount(void){
    char s[13];
    printf("Search AccNo: "); fflush(stdout);
    safeReadLine(s, sizeof s);
    int i = findIndexByAccNo(s);
    if(i >= 0) printf("[FOUND] %s | %s | %s | %.2f\n",
                      accounts[i].accNo, accounts[i].name,
                      typeToString(accounts[i].type), accounts[i].balance);
    else puts("[SEARCH] Not found.");
}


static void emiCalculator(void){
    double P = readDoublePrompt("Principal: ");
    double rate = readDoublePrompt("Rate%%: ");
    int n = readIntPrompt("Months: ");
    double r = rate / (12 * 100.0);
    double pw = pow1p(r, n);
    double emi = (P * r * pw) / (pw - 1);
    printf("EMI: %.2f | Total: %.2f | Interest: %.2f\n", emi, emi*n, emi*n - P);
}


static bool askPassword3(const char *prompt, const char *correct){
    char p[PASS_MAX_LEN];
    int tries = 3;
    while(tries > 0){
        printf("%s", prompt); fflush(stdout);
        safeReadLine(p, sizeof p);

        if(strlen(p) == 0){
            puts("Empty password. Try again.");
            continue;
        }

        if(strcmp(p, correct) == 0) return true;

        tries--;
        printf("Wrong. (%d tries left)\n", tries);
    }
    return false;
}
static bool userLogin(int *outIndex){
    char s[13];
    printf("AccNo: "); fflush(stdout);
    safeReadLine(s, sizeof s);
    int i = findIndexByAccNo(s); if(i < 0){ puts("Not found."); return false; }
    if(askPassword3("Pass: ", accounts[i].password)){ *outIndex = i; return true; }
    return false;
}
static bool adminLogin(void){
    char u[32];
    int tries = 3;
    while(tries > 0){
        printf("Admin user: "); fflush(stdout);
        safeReadLine(u, sizeof u);
        if(strcmp(u, "admin25") == 0 && askPassword3("Admin pass: ", "admin25")) return true;
        tries--;
        printf("Invalid admin. (%d tries left)\n", tries);
    }
    return false;
}

//Menus
static void userMenu(int i){
    for(;;){
        printf("\n-- User %s Menu --\n1)Deposit\n2)Withdraw\n3)Check Balance\n4)Exit\n", accounts[i].name);
        int c = readIntPrompt("Choice: ");
        if(c==1) depositAtIndex(i);
        else if(c==2) withdrawAtIndex(i);
        else if(c==3) printf("Balance: %.2f\n", accounts[i].balance);
        else if(c==4) break;
    }
}
static void adminMenu(void){
    for(;;){
        printf("\n-- Admin Menu --\n");
        printf("1)Add Account\n2)Update Account\n3)Delete Account\n4)Search\n5)Display All\n");
        printf("6)Sort by AccNo (MergeSort)\n7)Sort by Balance (QuickSort)\n");
        printf("8)Deposit\n9)Withdraw\n10)EMI Calc\n11)Exit\n");
        int c = readIntPrompt("Choice: ");
        if(c==1) addAccount();
        else if(c==2) updateAccount();
        else if(c==3) deleteAccount();
        else if(c==4) searchAccount();
        else if(c==5) displayAccounts();
        else if(c==6) sortByAccNo();
        else if(c==7) sortByBalance();
        else if(c==8) depositByAcc();
        else if(c==9) withdrawByAcc();
        else if(c==10) emiCalculator();
        else if(c==11) break;
    }
}

//Random demo accounts
static const char* sampleNames[] = {
    "Rahim","Karim","Hasan","Maruf","Anika","Jannat","Sadia","Rafi",
    "Imran","Nusrat","Ruhul","Mitu","Khalid","Shakib","Tanvir","Sumaiya","Ovi","Jubair"
};
static void randomRealName(char* out){
    int idx = rand() % (sizeof(sampleNames)/sizeof(sampleNames[0]));
    strncpy(out, sampleNames[idx], NAME_MAX_LEN);
    out[NAME_MAX_LEN] = '\0';
}
static void generateRandomAccounts(int n){
    for(int i=0;i<n && accountCount<MAX_ACCOUNTS;i++){
        Account a;
        generateAccountNumber(a.accNo);
        randomRealName(a.name);
        a.type = (AccType)(rand()%3+1);
        a.balance = (rand()%50000) + minBalanceForType(a.type);
        strcpy(a.password,"123456");
        accounts[accountCount++] = a;
    }
}

int main(void){
    srand((unsigned)time(NULL));
    generateRandomAccounts(20);
    saveAll();
    isInit = false;

    for(;;){
        printf("\n-- Main Menu --\n1)User\n2)Admin\n3)Exit\n");
        int c = readIntPrompt("Choice: ");
        if(c==1){ int i; if(userLogin(&i)) userMenu(i); }
        else if(c==2){ if(adminLogin()) adminMenu(); }
        else if(c==3) break;
    }
    return 0;
}
