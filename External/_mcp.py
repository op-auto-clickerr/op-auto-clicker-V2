import json,sys,urllib.request,re
URL="http://127.0.0.1:8225/mcp"
TOK="jXkmyZPD-M6d-PaMUbKM_h3nrlFNRyuAaMr_imHTtis"
def rpc(method,params=None,i=1):
    body=json.dumps({"jsonrpc":"2.0","id":i,"method":method,"params":params or {}}).encode()
    req=urllib.request.Request(URL,data=body,headers={"Content-Type":"application/json","Accept":"application/json, text/event-stream","Authorization":"Bearer "+TOK})
    raw=urllib.request.urlopen(req,timeout=120).read().decode("utf-8","ignore").strip()
    if raw.startswith("event:") or "\ndata:" in raw[:80]:
        raw="\n".join(l[len("data:"):].strip() for l in raw.splitlines() if l.startswith("data:"))
    return json.loads(raw)
def call(name,args):
    r=rpc("tools/call",{"name":name,"arguments":args})
    if "error" in r: return None,"RPC_ERROR "+json.dumps(r["error"])
    c=r.get("result",{}).get("content",[])
    return "".join(x.get("text","") for x in c), None
if sys.argv[1]=="tool":
    txt,err=call(sys.argv[2], json.loads(sys.argv[3]) if len(sys.argv)>3 else {})
    print(err or txt)
elif sys.argv[1]=="run":
    src=open(sys.argv[2],encoding="utf-8").read()
    a={"source":src}
    if len(sys.argv)>3: a["pid"]=sys.argv[3]
    txt,err=call("execute_script",a)
    print("EXEC:",err or txt)
    m=re.search(r"\b(\d{1,12})\b",txt or err or "")
    cur=int(m.group(1)) if m else 0
    import time; time.sleep(4)
    txt2,err2=call("read_console",{"after_cursor":cur,"limit":400,"wait_ms":3000})
    print("CONSOLE:",err2 or txt2)
