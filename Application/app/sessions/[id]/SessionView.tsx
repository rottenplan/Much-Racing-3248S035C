'use client';

import Link from 'next/link';
import { useState } from 'react';
import {
    ArrowLeft, Calendar, Share2, Download, MapPin,
    Settings, CheckSquare, Square, Play, Pause, RotateCw,
    ChevronDown, Maximize2, Flag, Activity
} from 'lucide-react';
import {
    LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, ReferenceLine
} from 'recharts';
import Navbar from "../../components/Navbar";
import SessionMapWrapper from "../../components/SessionMapWrapper";

interface SessionData {
    id: any;
    originalFilename: string;
    uploadDate: string;
    stats: any;
    points: any[];
}

export default function SessionView({ session }: { session: SessionData }) {
    const { points, id } = session;
    const [selectedLaps, setSelectedLaps] = useState<number[]>([1, 2, 4]); // Mock selection

    // Mock Laps Data (derived from session stats or static for now)
    const laps = [
        { id: 1, lap: 2, time: '01:02.405', gap: '+220 ms', distance: 620.1, maxSpeed: 82.1, maxRpm: 8400, color: '#ef4444' },
        { id: 2, lap: 4, time: '01:02.266', gap: '--', distance: 620.8, maxSpeed: 82.1, maxRpm: 8450, color: '#22c55e' }, // Best
        { id: 3, lap: 5, time: '01:02.397', gap: '+102 ms', distance: 617.3, maxSpeed: 82.2, maxRpm: 8380, color: '#3b82f6' },
        { id: 4, lap: 6, time: '01:03.347', gap: '+1081 ms', distance: 621.5, maxSpeed: 81.5, maxRpm: 8200, color: '#f59e0b' },
    ];

    // Format Chart Data
    // Use index as "time" or "distance" proxy for mockup
    const chartData = points.map((p, idx) => ({
        idx,
        time: idx * 0.1, // assuming 10hz
        speed: p.speed,
        rpm: p.rpm,
        // Mock Delta: simple sine wave + noise to look "data-like"
        delta: Math.sin(idx * 0.05) * 100 + (Math.random() * 20),
        waterTemp: 82 + Math.random() * 2,
    }));

    const mapPoints = points.map((p: any) => ({ lat: p.lat, lng: p.lng }));
    const dateStr = new Date(session.uploadDate).toLocaleString();

    return (
        <div className="min-h-screen bg-[#121212] text-white font-sans overflow-hidden flex flex-col">
            {/* Top Navigation (Minimal) */}
            <div className="bg-[#1e1e1e] h-8 flex items-center justify-end px-4 text-xs text-gray-400 border-b border-[#333]">
                <Link href="/dashboard" className="hover:text-white mr-4">Home</Link>
                <span className="mr-4">/</span>
                <span className="hover:text-white mr-4">FoxLap</span>
                <span className="mr-4">/</span>
                <span className="text-white">My Session - Data Addict Mode</span>
            </div>

            <div className="flex flex-1 overflow-hidden">
                {/* Left Sidebar */}
                <div className="w-80 bg-[#1e1e1e] flex flex-col border-r border-[#333]">
                    {/* Track Card */}
                    <div className="p-4 bg-[#0099cc] text-white relative h-32 flex flex-col justify-between">
                        <div className="absolute top-2 right-2">
                            <Settings size={16} className="cursor-pointer hover:rotate-90 transition" />
                        </div>
                        <div className="flex items-start gap-4">
                            {/* Map Icon Placeholder */}
                            <div className="w-12 h-12 border-2 border-white/30 rounded opacity-80">
                                {/* Simple SVG Track Shape */}
                                <svg viewBox="0 0 100 100" className="w-full h-full stroke-white fill-none stroke-2">
                                    <path d="M20,80 Q10,50 30,30 T70,30 T80,80 Z" />
                                </svg>
                            </div>
                            <div>
                                <h2 className="font-bold text-sm leading-tight">Sentul Internasional<br />Karting Cirkuit</h2>
                                <p className="text-[10px] opacity-80 mt-1">16111 BOGOR - Indonesia</p>
                            </div>
                        </div>
                        <div className="text-right text-2xl font-bold">1181m</div>
                    </div>

                    {/* Laps Header */}
                    <div className="bg-[#0077aa] px-4 py-1 flex items-center justify-between text-xs font-bold text-white uppercase">
                        <span>Laps</span>
                        <div className="flex gap-2">
                            <FilterIcon />
                            <Share2 size={12} />
                            <Download size={12} />
                        </div>
                    </div>

                    {/* Session Info Checkbox */}
                    <div className="p-2 border-b border-[#333]">
                        <div className="bg-[#0099cc] text-white text-xs p-2 rounded flex items-center justify-between">
                            <span>- {dateStr} (FARIS)</span>
                            <ChevronDown size={14} />
                        </div>
                    </div>

                    {/* Laps List */}
                    <div className="flex-1 overflow-y-auto p-2 space-y-1">
                        {laps.map((lap) => (
                            <div key={lap.id} className="flex items-center text-xs gap-2 p-1 hover:bg-[#333] rounded cursor-pointer">
                                <div
                                    className={`w-3 h-3 border rounded-sm flex items-center justify-center ${selectedLaps.includes(lap.id) ? 'bg-' + lap.color : 'border-gray-500'}`}
                                    style={{ backgroundColor: selectedLaps.includes(lap.id) ? lap.color : 'transparent', borderColor: lap.color }}
                                >
                                    {selectedLaps.includes(lap.id) && <div className="w-1.5 h-1.5 bg-white rounded-full" />}
                                </div>
                                <span className={`font-mono ${lap.gap === '--' ? 'text-green-400 font-bold' : 'text-gray-300'}`}>
                                    {String(lap.id).padStart(2, '0')} - {lap.time} ({lap.gap === '--' ? '+0 ms' : lap.gap})
                                </span>
                                <div className="ml-auto flex gap-1 opacity-50">
                                    <Activity size={10} />
                                </div>
                            </div>
                        ))}
                    </div>

                    {/* Footer Actions */}
                    <div className="p-2 mt-auto">
                        <button className="w-full bg-[#0077aa] hover:bg-[#006699] text-white py-2 text-xs font-bold rounded uppercase">
                            Load Another session
                        </button>
                    </div>
                </div>

                {/* Main Content Area */}
                <div className="flex-1 bg-[#121212] flex flex-col overflow-y-auto">

                    {/* Top Stats Panel */}
                    <div className="p-4">
                        <div className="bg-[#1e1e1e] border border-[#333] rounded-sm overflow-hidden">
                            {/* Timer Header */}
                            <div className="bg-black px-4 py-2 border-b border-[#333] flex justify-between items-center">
                                <span className="text-yellow-500 text-2xl font-mono font-bold">00:32.885</span>
                                <div className="flex gap-1">
                                    <ToolbarButton icon={<Square size={12} className="fill-orange-500 text-orange-500" />} />
                                    <ToolbarButton icon={<Play size={12} className="fill-yellow-500 text-yellow-500" />} />
                                    <ToolbarButton icon={<Flag size={12} />} />
                                    <ToolbarButton icon={<MapPin size={12} />} />
                                </div>
                            </div>

                            {/* Data Table */}
                            <div className="w-full text-xs">
                                <div className="grid grid-cols-7 bg-[#2d2d2d] text-gray-400 border-b border-[#333] py-1 px-4 font-bold uppercase">
                                    <div>Lap</div>
                                    <div>Time</div>
                                    <div>Distance</div>
                                    <div>Speed km/h</div>
                                    <div>RPM</div>
                                    <div>Water Temp</div>
                                    <div>Gap</div>
                                </div>

                                {laps.filter(l => selectedLaps.includes(l.id)).map(lap => (
                                    <div key={lap.id} className="grid grid-cols-7 py-2 px-4 border-b border-[#333] hover:bg-[#252525] items-center text-gray-300 font-mono">
                                        <div className="uppercase text-[#0099cc] font-bold">LAP {lap.lap}</div>
                                        <div>{lap.time}</div>
                                        <div>
                                            <div>{lap.distance.toFixed(1)}</div>
                                            <div className="h-1 bg-gray-700 w-16 mt-1 rounded-full overflow-hidden">
                                                <div className="h-full bg-orange-500" style={{ width: '80%' }}></div>
                                            </div>
                                        </div>
                                        <div>
                                            <div className="text-[#00ccff] font-bold">{lap.maxSpeed} Km/h</div>
                                            <div className="h-1 bg-gray-700 w-16 mt-1 rounded-full overflow-hidden">
                                                <div className="h-full bg-[#00ccff]" style={{ width: `${(lap.maxSpeed / 100) * 100}%` }}></div>
                                            </div>
                                        </div>
                                        <div>
                                            <div>--</div>
                                            <div className="h-1 bg-gray-700 w-16 mt-1 rounded-full overflow-hidden">
                                                <div className="h-full bg-purple-500" style={{ width: '40%' }}></div>
                                            </div>
                                        </div>
                                        <div>
                                            <div>--</div>
                                            <div className="h-1 bg-gray-700 w-16 mt-1 rounded-full overflow-hidden">
                                                <div className="h-full bg-red-500" style={{ width: '20%' }}></div>
                                            </div>
                                        </div>
                                        <div className={lap.gap === '--' ? "" : "text-[#00ccff]"}>{lap.gap}</div>
                                    </div>
                                ))}
                            </div>
                        </div>
                    </div>

                    {/* Charts Area */}
                    <div className="flex-1 px-4 pb-4 space-y-1">
                        {/* Speed Chart */}
                        <ChartBlock height={180} title="Speed (Km/h)">
                            <LineChart data={chartData}>
                                <CartesianGrid strokeDasharray="3 3" stroke="#333" vertical={false} />
                                <YAxis stroke="#666" fontSize={10} tickCount={5} domain={[0, 'auto']} />
                                <Tooltip content={<CustomTooltip />} />
                                <Line type="monotone" dataKey="speed" stroke="#00ccff" strokeWidth={1.5} dot={false} />
                                <Line type="monotone" dataKey="speed" stroke="#purple" strokeDasharray="3 3" strokeWidth={1} dot={false} strokeOpacity={0.5} />
                            </LineChart>
                        </ChartBlock>

                        {/* RPM Chart */}
                        <ChartBlock height={120} title="RPM">
                            <LineChart data={chartData}>
                                <CartesianGrid strokeDasharray="3 3" stroke="#333" vertical={false} />
                                <YAxis stroke="#666" fontSize={10} tickCount={3} domain={[0, 'auto']} />
                                <Tooltip content={<CustomTooltip />} />
                                <Line type="monotone" dataKey="rpm" stroke="#ff0000" strokeWidth={1.5} dot={false} />
                            </LineChart>
                        </ChartBlock>

                        {/* Delta Chart */}
                        <ChartBlock height={120} title="Delta (ms)">
                            <LineChart data={chartData}>
                                <CartesianGrid strokeDasharray="3 3" stroke="#333" vertical={false} />
                                <YAxis stroke="#666" fontSize={10} tickCount={3} />
                                <ReferenceLine y={0} stroke="#666" />
                                <Tooltip content={<CustomTooltip />} />
                                <Line type="monotone" dataKey="delta" stroke="#00ff00" strokeWidth={1.5} dot={false} />
                            </LineChart>
                        </ChartBlock>
                    </div>
                </div>

                {/* Right Side Map (Small, fixed or conditional) */}
                {/* In the reference, there's a small map overlay on the table, but I'll stick to maintaining layout cleanliness */}
            </div>
        </div>
    );
}

function ToolbarButton({ icon }: { icon: React.ReactNode }) {
    return (
        <button className="p-1 hover:bg-[#333] rounded text-gray-400 hover:text-white transition">
            {icon}
        </button>
    );
}

function FilterIcon() {
    return (
        <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <polygon points="22 3 2 3 10 12.46 10 19 14 21 14 12.46 22 3"></polygon>
        </svg>
    )
}

function ChartBlock({ children, height, title }: { children: React.ReactNode, height: number, title?: string }) {
    return (
        <div className="relative w-full bg-[#1a1a1a]" style={{ height }}>
            {title && <div className="absolute top-2 left-10 text-[10px] text-gray-500 font-bold z-10 rotate-[-90deg] origin-left translate-y-4">{title}</div>}
            <ResponsiveContainer width="100%" height="100%">
                {children as any}
            </ResponsiveContainer>
        </div>
    )
}

const CustomTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
        return (
            <div className="bg-[#1e1e1e] border border-[#333] p-2 text-xs text-white shadow-xl">
                <p className="font-mono">{`Time: ${label}`}</p>
                {payload.map((p: any) => (
                    <p key={p.name} style={{ color: p.color }}>
                        {`${p.name}: ${Number(p.value).toFixed(1)}`}
                    </p>
                ))}
            </div>
        );
    }
    return null;
};
