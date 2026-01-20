'use client';

import { Activity, Clock, Timer, Zap } from "lucide-react";
import { useLiveTelemetry } from "../hooks/useLiveTelemetry";
import Speedometer from "./Speedometer";

// Mock data for now - will hook up to firmware later
const dragStats = {
    zeroToSixty: "--.--",
    zeroToHundred: "--.--",
    hundredToTwoHundred: "--.--",
    quarterMile: "--.--",
    eighthMile: "--.--"
};

export default function DragModeView() {
    const { data, ip, setIp, connected } = useLiveTelemetry();

    return (
        <div className="w-full h-full flex flex-col p-4 relative">
            {/* Connection Status & IP Control (Top Right Aboslute) */}
            <div className="absolute top-4 right-4 z-10 flex gap-2 items-center">
                <span className={`w-3 h-3 rounded-full ${connected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`}></span>
                <input
                    type="text"
                    value={ip}
                    onChange={(e) => setIp(e.target.value)}
                    className="bg-black/50 text-white px-2 py-1 rounded border border-gray-700 w-28 text-xs backdrop-blur-sm"
                    placeholder="Device IP"
                />
            </div>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-6 h-full">
                {/* Left Side: BIG Speedometer */}
                <div className="flex flex-col items-center justify-center relative p-6 bg-background-secondary/30 rounded-2xl border border-white/5">
                    <Speedometer />
                    <div className="mt-[-40px] text-center z-10">
                        <div className="text-6xl font-bold font-data text-white tabular-nums tracking-tighter shadow-glow-blue">
                            {Math.round(data.speed)}
                        </div>
                        <div className="text-xl font-racing text-primary tracking-widest mt-2">KM/H</div>
                    </div>
                </div>

                {/* Right Side: Drag Times Grid */}
                <div className="grid grid-cols-1 gap-3 content-center">
                    <DragStatRow label="0 - 60 KM/H" time={dragStats.zeroToSixty} icon={<Zap className="w-4 h-4 text-warning" />} />
                    <DragStatRow label="0 - 100 KM/H" time={dragStats.zeroToHundred} icon={<Activity className="w-4 h-4 text-primary" />} active />
                    <DragStatRow label="100 - 200 KM/H" time={dragStats.hundredToTwoHundred} icon={<Timer className="w-4 h-4 text-highlight" />} />
                    <div className="my-2 border-t border-white/10"></div>
                    <DragStatRow label="201m (1/8 Mile)" time={dragStats.eighthMile} icon={<Clock className="w-4 h-4 text-text-secondary" />} />
                    <DragStatRow label="402m (1/4 Mile)" time={dragStats.quarterMile} icon={<Clock className="w-4 h-4 text-text-secondary" />} />
                </div>
            </div>
        </div>
    );
}

function DragStatRow({ label, time, icon, active = false }: { label: string, time: string, icon: React.ReactNode, active?: boolean }) {
    return (
        <div className={`
            flex items-center justify-between p-4 rounded-xl border transition-all duration-300
            ${active
                ? 'bg-primary/10 border-primary shadow-[0_0_15px_rgba(59,130,246,0.3)]'
                : 'bg-background-secondary/50 border-white/5 hover:border-white/10'
            }
        `}>
            <div className="flex items-center gap-3">
                <div className={`p-2 rounded-lg ${active ? 'bg-primary/20' : 'bg-black/30'}`}>
                    {icon}
                </div>
                <span className={`font-racing text-sm ${active ? 'text-white' : 'text-text-secondary'}`}>
                    {label}
                </span>
            </div>
            <div className={`font-data text-2xl font-bold ${active ? 'text-primary' : 'text-white'}`}>
                {time} <span className="text-xs text-text-secondary font-normal ml-1">s</span>
            </div>
        </div>
    );
}
