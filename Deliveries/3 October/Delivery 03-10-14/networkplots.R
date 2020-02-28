library("fastmatch")
library("gsubfn")
library("igraph")
setwd("~/Dropbox/Tesis/Code")
mydata <- read.table("network.txt", header=TRUE, sep=",",fill=T)

pause <- function() {
  cat("Press ENTER to continue.")
  readLines(n=1)
  invisible()
}

plotnetwork<-function(Time){
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
  
  
  degree(g,mode = "total")
  #clusters(g)
  #get.adjacency(g)
  #pause()
}

###########################################################################################

for(t in c(1:100)){
  plotnetwork(t)
  pause()
}

for(t in c(1:2000)){
  png(paste("GRAFICOS/filename",formatC(t, width = 5, format = "d", flag = "0")
 ,".png",sep=""))
  plotnetwork(t)
  dev.off()
  }

####Closeness Series
closenessData=0
for(t in c(1:2000)){
closenessData = rbind(closenessData,networkCloseness(t))
}

plot(mydata$Time[1:4001],closenessData[,1],type="l",ylim=c(0,0.1))
for(i in c(2:10)){
lines(mydata$Time[1:4001],closenessData[,i],col=i)
}
legend("top",legend=0:9,col=1:10,lty=1,horiz=T)

####Degree Series
DegreeData=0
for(t in c(1:4000)){
  DegreeData = rbind(DegreeData,networkDegree(t))
}


plot(mydata$Time[1:4001],DegreeData[,1],type="l",ylim=c(0,15),ylab="Degree",xlab="Time")
for(i in c(2:10)){
  lines(mydata$Time[1:4001],DegreeData[,i],col=i)
}
legend("top",legend=0:9,col=1:10,lty=1,horiz=T)


