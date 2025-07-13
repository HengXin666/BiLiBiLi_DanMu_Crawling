import { useState } from "react";

function App() {
    const [progress, setProgress] = useState<string>("")

    const startTask = async () => {
        const res = await fetch("http://localhost:8000/start-task", { method: "POST" })
        const data = await res.json()
        const taskId = data.taskId

        const ws = new WebSocket(`ws://localhost:8000/ws/${taskId}`);
        ws.onopen = () => {
            console.log("WebSocket connected");
        };
        ws.onmessage = (event) => {
            console.log("Received:", event.data);
            setProgress(event.data);
        };
        ws.onerror = (e) => {
            console.error("WebSocket error", e);
        };
        ws.onclose = () => {
            console.log("WebSocket closed");
        };
    }

    return (
        <div style={{ padding: 20 }}>
            <button onClick={startTask}>开始任务</button>
            <div>进度: {progress}</div>
        </div>
    )
}

export default App
