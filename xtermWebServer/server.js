const WebSocket = require("ws");
const { spawn } = require("child_process");

const PORT = 8080;
const MAX_CLIENTS = 50;

let activeConnections = 0;

const wss = new WebSocket.Server({ port: PORT });

wss.on("connection", (ws) => {
    if (activeConnections >= MAX_CLIENTS) {
        ws.send("Server busy. Try again later.\n");
        ws.close();
        return;
    }

    activeConnections++;

    const client = spawn("script", ["-qfc", "../client", "/dev/null"]);

    client.stdout.setEncoding("utf-8");
    client.stderr.setEncoding("utf-8");

    ws.send("Connected to C client\r\n");

    ws.on("message", (msg) => {
        client.stdin.write(msg.toString());
    });

    client.stdout.on("data", (data) => {
        ws.send(data);
    });

    client.stderr.on("data", (data) => {
        ws.send(data);
    });

    const cleanup = () => {
        if (!client.killed) client.kill("SIGKILL");
        activeConnections--;
    };

    ws.on("close", cleanup);
    ws.on("error", cleanup);

    client.on("close", () => {
        ws.close();
    });
});

console.log(`Running at ws://localhost:${PORT}`);