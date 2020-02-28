setwd("~/Dropbox/Tesis/Code")
source("./networkplots.R")

#7-9-5-8-11-12

for(t in c(1:400)){
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
Ndata =length(mydata$Time)
NBanks = 100
for(t in c(1:Ndata)){
  DegreeData = rbind(DegreeData,networkDegree(t))
}

DegreeData[,][DegreeData[,]==0]<-NA

#DegreeData[,][DegreeData[,]==NA]<-0

for(t in c(1:Ndata)){
  mydata$meanDegree[t] = mean(DegreeData[t,],na.rm=T)
  mydata$stdDegree[t] = sd(DegreeData[t,],na.rm=T)
}

plot(mydata$Time[1:(Ndata+1)],DegreeData[,1],type="l",ylim=c(0,15),ylab="Total Degree",xlab="Time")
for(i in c(2:NBanks)){
  lines(mydata$Time[1:(Ndata+1)],DegreeData[,i],col=i)
}
lines(mydata$Time[1:Ndata],mydata$meanDegree,pch=22,lwd=3)
lines(mydata$Time[1:Ndata],mydata$meanDegree+2*mydata$stdDegree,lwd=3)
lines(mydata$Time[1:Ndata],mydata$meanDegree-2*mydata$stdDegree,lwd=3)


legend("top",legend=0:9,col=1:10,lty=1,horiz=T)
