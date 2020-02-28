library("fastmatch")
library("gsubfn")
library("igraph")
setwd("~/Dropbox/Tesis/Code")
mydata <- read.table("./Debug/network.txt", header=TRUE, sep=",",fill=T)

pause <- function() {
  cat("Press ENTER to continue.")
  readLines(n=1)
  invisible()
}

N=120
plotnetwork<-function(Time){
banks=vector(length=0)
i=0
while(i<=N){
  banks = c(banks, toString(i))
i=i+1}

T=Time
first.bank.col = fmatch("Partners.Bank.0",names(mydata))
banks.to=vector(length=0)
banks.from=vector(length=0)
n=0
while(n<=N){
info = mydata[T,first.bank.col+n]
aux.to=gsub("\\D", " ", info)
aux.to=gsub(" $", "", aux.to)
aux.to=strapply(aux.to, '\\d+', c , simplify = cbind)
aux.to=as.vector(aux.to)
aux.from=vector(length = length(aux.to))
aux.from[]=toString(n)
aux.to
aux.from
banks.to=c(banks.to,aux.to)
banks.from=c(banks.from,aux.from)
n=n+1
}

banks.to
banks.from


banks <- data.frame(name=banks)
relations <- data.frame(from=banks.from, to=banks.to)
g <- graph.data.frame(relations, directed=TRUE, vertices=banks)
plot(g,vertex.color=NA,vertex.label.color="black", layout = layout.circle(g),main=paste("Row",toString(T),"= Time",mydata$Time[T]))


closeness(g)
#clusters(g)
#get.adjacency(g)
#pause()
}
##########################CLOSENESS
networkCloseness<-function(Time){
  banks=vector(length=0)
  i=0
  while(i<=9){
    banks = c(banks, toString(i))
    i=i+1}
  
  T=Time
  first.bank.col = fmatch("Partners.Bank.0",names(mydata))
  banks.to=vector(length=0)
  banks.from=vector(length=0)
  n=0
  while(n<=9){
    info = mydata[T,first.bank.col+n]
    aux.to=gsub("\\D", " ", info)
    aux.to=gsub(" $", "", aux.to)
    aux.to=strapply(aux.to, '\\d+', c , simplify = cbind)
    aux.to=as.vector(aux.to)
    aux.from=vector(length = length(aux.to))
    aux.from[]=toString(n)
    aux.to
    aux.from
    banks.to=c(banks.to,aux.to)
    banks.from=c(banks.from,aux.from)
    n=n+1
  }
  
  banks.to
  banks.from
  
  
  banks <- data.frame(name=banks)
  relations <- data.frame(from=banks.from, to=banks.to)
  g <- graph.data.frame(relations, directed=TRUE, vertices=banks)
 # plot(g,vertex.color=NA,vertex.label.color="black", layout = layout.circle(g),main=paste("Row",toString(T),"= Time",mydata$Time[T]))
  
  
  closeness(g)
  #clusters(g)
  #get.adjacency(g)
  #pause()
}

#####DEGREE
networkDegree<-function(Time){
  banks=vector(length=0)
  i=0
  while(i<=N){
    banks = c(banks, toString(i))
    i=i+1}
  
  T=Time
  first.bank.col = fmatch("Partners.Bank.0",names(mydata))
  banks.to=vector(length=0)
  banks.from=vector(length=0)
  n=0
  while(n<=N){
    info = mydata[T,first.bank.col+n]
    aux.to=gsub("\\D", " ", info)
    aux.to=gsub(" $", "", aux.to)
    aux.to=strapply(aux.to, '\\d+', c , simplify = cbind)
    aux.to=as.vector(aux.to)
    aux.from=vector(length = length(aux.to))
    aux.from[]=toString(n)
    aux.to
    aux.from
    banks.to=c(banks.to,aux.to)
    banks.from=c(banks.from,aux.from)
    n=n+1
  }
  
  banks.to
  banks.from
  
  
  banks <- data.frame(name=banks)
  relations <- data.frame(from=banks.from, to=banks.to)
  g <- graph.data.frame(relations, directed=TRUE, vertices=banks)
  # plot(g,vertex.color=NA,vertex.label.color="black", layout = layout.circle(g),main=paste("Row",toString(T),"= Time",mydata$Time[T]))
  
  
  degree(g,mode = "total")
  #clusters(g)
  #get.adjacency(g)
  #pause()
}

###########################################################################################



