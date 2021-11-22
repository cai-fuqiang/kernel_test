```
1. 2.1.2.1
IA-32e(64位模式)不支持 task gates
在特权级改变的时候，stack segment selectors 将不再从
TSS中读取。另外，他们被设置成NULL
```

