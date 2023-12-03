t=0:(1/44100):0.2;
c=chirp(t,2000,0.2,8000);
len=length(t);
input_seq=[zeros(1,len),c,zeros(1,len)];
out=conv(input_seq,c(1:-1:end));
plot(out)