setwd("~/Dropbox/Tesis/Code")
mydata <- read.table("hola.txt", header=TRUE, sep=",",fill=T)
ListenerOutput <- read.table("ListenerOutput.txt", header=TRUE, sep=",",fill=T)
hist(ListenerOutput$IDborrower,breaks=100)
hist(ListenerOutput$IDlender,breaks=100)
plot(ListenerOutput$getID)
hist(ListenerOutput$GetLinkedAgentID,breaks=100)
hist(ListenerOutput$IsExpired,breaks=100)
hist(ListenerOutput$getType,breaks=100)

hist(ListenerOutput$GetLinkedAgentID[ListenerOutput$GetLinkedAgentID>0],breaks=100)

####IB DEPOSITS accepted
first.bank.col.IBdepositsA = fmatch("IB.Deposits.Accepted.Bank.0",names(mydata))
plot( mydata[,first.bank.col.IBdepositsA], type="l", main = "IB.Deposits.Accepted.Bank",ylim=c(0,5))
n=1
while(n<=9){
  lines(mydata[,first.bank.col.IBdepositsA+n],col=n)
  n=n+1
}

####IB DEPOSITS requested
first.bank.col.IBdepositsR = fmatch("IB.Deposits.Requested.Bank.0",names(mydata))
plot( mydata[,first.bank.col.IBdepositsR], type="l", main = "IB.Deposits.Requested.Bank")
n=1
while(n<=9){
  lines(mydata[,first.bank.col.IBdepositsR+n],col=n)
  n=n+1
}

####CUSTOMER DEPOSITS
first.bank.col.deposits = fmatch("Customer.Deposit.Bank.0",names(mydata))
plot( mydata[,first.bank.col.deposits], type="l", main = "Customer.Deposit.arrivals",ylim=c(0,100))
n=1
while(n<=9){
  lines(mydata[,first.bank.col.deposits+n],col=n)
  n=n+1
}
####CUSTOMER LOANS
first.bank.col.loans = fmatch("Customer.Loan.Bank.0",names(mydata))
plot( mydata[,first.bank.col.loans], type="l", main = "Customer.Loan.arrivals",ylim=c(0,500))
n=1
while(n<=9){
  lines(mydata[,first.bank.col.loans+n],col=n)
  n=n+1
}
#######


plot(t,mydata[[6]],col=1,xlab="Time",ylab="Value Last State-Action")

####EQUITY
mydata <- read.table("hola.txt", header=TRUE, sep=",",fill=T)
first.bank.col.equity = fmatch("Equity.Bank.0",names(mydata))
plot( mydata[,first.bank.col.equity], type="l", main = "Bank equity",ylim=c(0,300))
n=1
##n<=9 to show only initial banks
while(n<=20){
  lines(mydata[,first.bank.col.equity+n],col=n)
  n=n+1
}
#######



plot(t,n,type="l")
abline(h=1)

#queue_bank_2=mydata[[6]]
#queue_bank_3=mydata[[7]]
#queue_bank_3[queue_bank_3>1249705156]<-0
#sum_queue=queue_bank_1+queue_bank_2+queue_bank_3
plot(t,mydata[[4]],col="red",type="l")
points(t,queue_bank_1,type="l")
#points(t,queue_bank_2,type="l",col="blue")
#points(t,queue_bank_3,type="l",col="green")
#points(t,sum_queue,col="navy")

