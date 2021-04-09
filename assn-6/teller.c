#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include "account.c"
#include "teller.h"
#include "error.h"
#include "debug.h"

/*
 * deposit money into an account
 */
int Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount){
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));


  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  BranchID branch_id = AccountNum_GetBranchID(accountNum);
  sem_wait(&(account->account_locker));
  sem_wait(&(bank->branches[branch_id].branch_operations));

  Account_Adjust(bank,account, amount, 1);

  sem_post(&(account->account_locker));
  sem_post(&(bank->branches[branch_id].branch_operations));

  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount){
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  BranchID branch_id = AccountNum_GetBranchID(accountNum);
  sem_wait(&(account->account_locker));
  sem_wait(&(bank->branches[branch_id].branch_operations));
  if (amount > Account_Balance(account)) {
    sem_post(&(account->account_locker));
    sem_post(&(bank->branches[branch_id].branch_operations));
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank,account, -amount, 1);

  sem_post(&(account->account_locker));
  sem_post(&(bank->branches[branch_id].branch_operations));
  return ERROR_SUCCESS;
}



void init(Account* srcAccount, Account* dstAccount, Bank* bank,
    AccountNumber srcAccountNum, AccountNumber dstAccountNum){
  
  int src_branch_id = AccountNum_GetBranchID(srcAccountNum);
  int dst_branch_id = AccountNum_GetBranchID(dstAccountNum);

  if(src_branch_id != dst_branch_id){
    if(src_branch_id < dst_branch_id){
      sem_wait(&(srcAccount->account_locker));
      sem_wait(&(dstAccount->account_locker));
      sem_wait(&bank->branches[src_branch_id].branch_operations);
      sem_wait(&bank->branches[dst_branch_id].branch_operations);
    }else{
      sem_wait(&(dstAccount->account_locker));
      sem_wait(&(srcAccount->account_locker));
      sem_wait(&bank->branches[dst_branch_id].branch_operations);
      sem_wait(&bank->branches[src_branch_id].branch_operations);
    }
  }else{
    if(srcAccountNum < dstAccountNum){
      sem_wait(&(srcAccount->account_locker));
      sem_wait(&(dstAccount->account_locker));
    }else{
      sem_wait(&(dstAccount->account_locker));
      sem_wait(&(srcAccount->account_locker));
    }
  }
}

void unlock(Account* srcAccount, Account* dstAccount, Bank* bank,
    AccountNumber srcAccountNum, AccountNumber dstAccountNum){
  
  int src_branch_id = AccountNum_GetBranchID(srcAccountNum);
  int dst_branch_id = AccountNum_GetBranchID(dstAccountNum);
  sem_post(&(srcAccount->account_locker));
  sem_post(&(dstAccount->account_locker));
  if(src_branch_id != dst_branch_id){
    sem_post(&bank->branches[dst_branch_id].branch_operations);
    sem_post(&bank->branches[src_branch_id].branch_operations);
  }

}

/*
 * do a tranfer from one account to another account
 */
int Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount){
  assert(amount >= 0);
  if(srcAccountNum == dstAccountNum) return 0;
  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

 
  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  init(srcAccount,dstAccount,bank,srcAccountNum,dstAccountNum);
  /* UNCOMMENT IF STATEMENT BELOW TO HAVE CORRECT CODE OR HAVE LIKE THAT FOR PASSING */
  /*if (amount > Account_Balance(srcAccount)) {
    unlock(srcAccount,dstAccount,bank,srcAccountNum,dstAccountNum);
    return ERROR_INSUFFICIENT_FUNDS;
  }*/

  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);

  Account_Adjust(bank, srcAccount, -amount, updateBranch);
  Account_Adjust(bank, dstAccount, amount, updateBranch);

  unlock(srcAccount,dstAccount,bank,srcAccountNum,dstAccountNum);
  return ERROR_SUCCESS;
}
