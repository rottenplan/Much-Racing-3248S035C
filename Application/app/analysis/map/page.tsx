"use client";

import {
    ChevronLeft,
    MoreHorizontal,
    Layers,
    ChevronRight
} from "lucide-react";
import Link from "next/link";
import { useRouter } from "next/navigation";

export default function MapAnalysisPage() {
    const router = useRouter();

    return (
        <main className="min-h-screen bg-white text-zinc-900 flex flex-col font-sans pb-safe relative overflow-hidden">
            {/* Header */}
            <div className="flex items-center justify-between p-4 bg-white border-b border-gray-100 z-10 sticky top-0">
                <button onClick={() => router.back()}>
                    <ChevronLeft className="w-8 h-8 text-red-600" />
                </button>
                <div className="text-center">
                    <h1 className="text-zinc-500 font-medium">Lap 05</h1>
                </div>
                <div className="w-8 flex justify-center">
                    <MoreHorizontal className="w-6 h-6 text-red-600" />
                </div>
            </div>

            {/* Lap Header */}
            <div className="flex items-center justify-between px-8 py-4 bg-white z-10">
                <ChevronLeft className="w-6 h-6 text-zinc-300" />
                <span className="text-3xl font-bold font-mono text-zinc-800">01:26.37</span>
                <ChevronRight className="w-6 h-6 text-zinc-300" />
            </div>

            {/* Satellite Map Area */}
            <div className="flex-1 relative bg-[#3f4a3c] overflow-hidden min-h-[400px]">
                {/* Satellite Texture Mock */}
                <div className="absolute inset-0 opacity-30 mix-blend-overlay"
                    style={{ backgroundImage: 'url("data:image/svg+xml,%3Csvg width=\'100\' height=\'100\' viewBox=\'0 0 100 100\' xmlns=\'http://www.w3.org/2000/svg\'%3E%3Cfilter id=\'noise\'%3E%3CfeTurbulence type=\'fractalNoise\' baseFrequency=\'0.8\' numOctaves=\'3\'/%3E%3C/filter%3E%3Crect width=\'100\' height=\'100\' filter=\'url(%23noise)\' opacity=\'0.5\'/%3E%3C/svg%3E")' }}>
                </div>

                {/* Map Content SVG */}
                <svg className="absolute inset-0 w-full h-full" viewBox="0 0 400 500" preserveAspectRatio="xMidYMid slice">
                    {/* Track "Asphalt" Outline */}
                    <path
                        d="M 50,150 C 50,150 150,200 180,250 S 200,400 150,500 L 300,450 C 350,400 350,300 300,250 L 200,100 Z"
                        fill="none" stroke="#222" strokeWidth="60" strokeLinecap="round" strokeLinejoin="round"
                        transform="scale(0.8) translate(50, 50)"
                        className="opacity-50"
                    />

                    {/* Racing Line (Blue) */}
                    <path
                        d="M 50,150 C 50,150 150,200 180,250 S 200,400 150,500 L 300,450 C 350,400 350,300 300,250 L 200,100 Z"
                        fill="none" stroke="#3b82f6" strokeWidth="4" strokeLinecap="round" strokeLinejoin="round"
                        transform="scale(0.8) translate(50, 50)"
                        className="opacity-90"
                    />

                    {/* Cursor Dot */}
                    <circle cx="180" cy="280" r="10" fill="#3b82f6" stroke="white" strokeWidth="3" className="shadow-lg" />
                </svg>

                {/* Google Logo Mock */}
                <div className="absolute bottom-4 left-4 text-white text-lg font-bold drop-shadow-md select-none opacity-80 font-sans tracking-tighter">
                    Google
                </div>

                <div className="absolute top-4 right-4">
                    <Layers className="w-10 h-10 text-red-600 bg-white rounded-full p-2 shadow-md cursor-pointer hover:bg-gray-50 transition-colors" />
                </div>
            </div>

            {/* Graph Sheet */}
            <div className="bg-white rounded-t-3xl shadow-[0_-5px_20px_rgba(0,0,0,0.1)] p-4 pb-8 z-20 relative -mt-6">

                {/* Cursor Label */}
                <div className="absolute -top-3 left-1/2 -translate-x-1/2">
                    <div className="bg-white px-2 rounded-full shadow-sm">
                        <span className="bg-red-50 text-red-600 text-xs font-bold px-2 py-0.5 rounded border border-red-200">
                            119.7
                        </span>
                    </div>
                    <div className="w-[1px] h-36 bg-red-400 absolute left-1/2 top-4"></div>
                </div>

                {/* Graph Area */}
                <div className="h-44 w-full relative mt-4 border-b border-gray-200">
                    {/* Y-Axis Labels */}
                    <div className="absolute left-0 top-0 bottom-0 flex flex-col justify-between text-[10px] text-red-600 font-bold font-mono py-2">
                        <span>175</span>
                        <span>140</span>
                        <span>105</span>
                        <span>70</span>
                        <span>35</span>
                    </div>

                    {/* Mock Graph Line */}
                    <svg className="absolute ml-8 inset-0 w-[calc(100%-32px)] h-full overflow-visible" preserveAspectRatio="none">
                        <line x1="0" y1="50%" x2="100%" y2="50%" stroke="#e5e7eb" strokeWidth="1" />

                        {/* Speed Line Mock (Red) */}
                        <path
                            d="M 0,20 L 20,25 L 40,80 L 60,70 L 80,70 L 100,50 L 120,40 L 140,80 L 160,70 L 180,60 L 200,40 L 220,70 L 240,80 L 260,60 L 280,40 L 300,50"
                            fill="none" stroke="#ef4444" strokeWidth="2" strokeLinejoin="round"
                        />
                        {/* Secondary Lines (G-Forces) */}
                        <path
                            d="M 0,60 Q 30,50 60,60 T 120,40 T 180,60 L 300,55"
                            fill="none" stroke="#a78bfa" strokeWidth="1" opacity="0.6"
                        />
                        <path
                            d="M 0,40 Q 40,45 80,40 T 160,35 T 240,40 L 300,45"
                            fill="none" stroke="#86efac" strokeWidth="1" opacity="0.6"
                        />
                    </svg>

                    {/* X-Axis Labels */}
                    <div className="absolute left-8 right-0 -bottom-5 flex justify-between text-[10px] text-gray-400 font-mono">
                        <span>00.0</span>
                        <span>25.6</span>
                        <span className="bg-red-500 text-white px-1 rounded">36.12</span>
                        <span>51.2</span>
                        <span>01:16.8</span>
                    </div>
                </div>

                {/* Legend */}
                <div className="flex justify-center flex-wrap gap-4 mt-8">
                    <LegendItem label="Speed (kph)" color="bg-gray-400" active />
                    <LegendItem label="Acceleration G" color="bg-green-500" />
                    <LegendItem label="Cornering G" color="bg-blue-600" />
                </div>

                {/* Footer Toggles */}
                <div className="flex gap-4 mt-6">
                    <button className="flex-1 py-3 rounded-full border border-gray-300 text-gray-600 font-bold text-sm">Summary</button>
                    <button className="flex-1 py-3 rounded-full border border-gray-300 text-gray-600 font-bold text-sm">Compare</button>
                </div>
            </div>
        </main>
    );
}

function LegendItem({ label, color, active }: { label: string, color: string, active?: boolean }) {
    return (
        <div className={`flex items-center gap-2 ${active ? 'bg-gray-100 px-3 py-1 rounded-full' : ''}`}>
            <div className={`w-3 h-3 rounded-full ${color}`}></div>
            <span className={`text-xs font-bold ${active ? 'text-zinc-800' : 'text-gray-400'}`}>{label}</span>
        </div>
    )
}
