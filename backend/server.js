const http = require("http");
const WebSocket = require("ws");
const fs = require("fs");
const dbConnection  = require("./db");
const PORT = process.env.PORT||5000;
dbConnection();
const server = http.createServer(async (req,res)=>{
    if(req.url==="/" && req.method === "GET"){
        fs.readFile('index.html',(error,pgResp)=>{
            if(!error){
                res.writeHead(200,{"Content-Type":"text/html"});
                res.write(pgResp);
                res.end();
            }
        });
        
    }else{
        res.writeHead(404,{"Content-Type":"application/json"});
        res.end(JSON.stringify({message:"Route not found"}));
    }
});

const wss = new WebSocket.Server({server});
wss.on('connection',function connection(ws){
    console.log("new connection");
    ws.on('message',function incoming(data){
        console.log(data.toString())
        wss.clients.forEach((client)=>{
            if(client!=ws){
                client.send(data.toString());
            }

        })
        
    })
})
server.listen(PORT,()=>{
    console.log(`server started at ${PORT}`);
});