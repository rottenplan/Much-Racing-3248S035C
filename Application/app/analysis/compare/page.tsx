"use client";

import {
    ChevronLeft,
    MoreHorizontal,
    Layers,
    ChevronDown
} from "lucide-react";
import Link from "next/link";
import { useRouter } from "next/navigation";
import { useState } from "react";

export default function LapComparePage() {
    const router = useRouter();
    const [graphTab, setGraphTab] = useState<'TIME' | 'SPEED' | 'G-FORCE'>('TIME');

    return (
        <main className="min-h-screen bg-white text-zinc-900 flex flex-col font-sans pb-safe relative overflow-hidden">
            {/* Header */}
            <div className="flex items-center justify-between p-4 bg-white border-b border-gray-100 z-10">
                <button onClick={() => router.back()}>
                    <ChevronLeft className="w-8 h-8 text-red-600" />
                </button>
                <h1 className="text-zinc-500 font-medium">Lap 05 vs Lap 03</h1>
                <div className="w-8 flex justify-center">
                    <MoreHorizontal className="w-6 h-6 text-red-600" />
                </div>
            </div>

            {/* Lap Selectors */}
            <div className="flex justify-between items-center px-4 py-2 bg-white shadow-sm z-10 relative">
                <button className="flex items-center gap-1">
                    <ChevronDown className="w-4 h-4 text-zinc-400" />
                    <span className="text-blue-600 font-bold font-mono text-xl">01:26.37</span>
                </button>
                <div className="bg-gray-200 rounded-full p-1">
                    <span className="text-[10px] font-bold text-gray-500 px-1">VS</span>
                </div>
                <button className="flex items-center gap-1">
                    <span className="text-red-600 font-bold font-mono text-xl">01:28.85</span>
                    <ChevronDown className="w-4 h-4 text-zinc-400" />
                </button>
            </div>

            {/* Map Area */}
            <div className="flex-1 relative bg-gray-50 overflow-hidden">
                {/* Map Grid */}
                <div className="absolute inset-0 grid grid-cols-[repeat(20,1fr)] grid-rows-[repeat(20,1fr)] opacity-10 pointer-events-none">
                    {[...Array(400)].map((_, i) => <div key={i} className="border border-gray-300"></div>)}
                </div>

                {/* Mock Map SVG */}
                <svg className="absolute inset-0 w-full h-full" viewBox="0 0 400 600" preserveAspectRatio="xMidYMid slice">
                    {/* Track Surface Mock */}
                    <path
                        d="M 50,200 C 50,200 150,250 180,300 S 200,450 150,550"
                        fill="none" stroke="#e5e7eb" strokeWidth="60" strokeLinecap="round"
                    />

                    {/* Lap Blue (Faster) */}
                    <path
                        d="M 50,200 C 50,200 150,250 178,300 S 198,450 150,550"
                        fill="none" stroke="#3b82f6" strokeWidth="4" strokeLinecap="round"
                        className="opacity-80"
                    />
                    {/* Lap Red (Slower) */}
                    <path
                        d="M 54,204 C 54,204 154,254 184,300 S 204,454 154,554"
                        fill="none" stroke="#ef4444" strokeWidth="4" strokeDasharray="3 3"
                        className="opacity-80"
                    />

                    {/* Cursor Dot */}
                    <circle cx="178" cy="300" r="8" fill="#3b82f6" stroke="white" strokeWidth="2" />
                    <circle cx="184" cy="300" r="8" fill="#ef4444" stroke="white" strokeWidth="2" className="opacity-80" />

                    {/* Crosshair Line */}
                    <line x1="160" y1="280" x2="200" y2="320" stroke="black" strokeWidth="1" />
                </svg>

                <div className="absolute top-4 right-4">
                    <Layers className="w-8 h-8 text-red-600 bg-white rounded-full p-1.5 shadow-md" />
                </div>
            </div>

            {/* Graph Sheet */}
            <div className="bg-white rounded-t-3xl shadow-[0_-5px_20px_rgba(0,0,0,0.1)] p-4 pb-8 z-20 relative -mt-4">
                {/* Delta Bubble */}
                <div className="absolute -top-3 left-1/2 -translate-x-1/2">
                    <div className="bg-white px-2 rounded-full shadow-sm">
                        <span className="bg-red-100 text-red-600 text-xs font-bold px-2 py-0.5 rounded border border-red-200">
                            +0.70
                        </span>
                    </div>
                    <div className="w-[1px] h-32 bg-red-400 absolute left-1/2 top-4"></div>
                </div>

                {/* Graph Area */}
                <div className="h-40 w-full relative mt-4 border-b border-gray-200">
                    {/* Y-Axis Labels */}
                    <div className="absolute left-0 top-0 bottom-0 flex flex-col justify-between text-[10px] text-gray-400 font-mono">
                        <span>+2.00</span>
                        <span>+1.00</span>
                        <span className="text-blue-500">0.00</span>
                        <span>-1.00</span>
                        <span>-2.00</span>
                    </div>

                    {/* Mock Graph Line */}
                    <svg className="absolute ml-10 inset-0 w-[calc(100%-40px)] h-full overflow-visible" preserveAspectRatio="none">
                        <line x1="0" y1="50%" x2="100%" y2="50%" stroke="#3b82f6" strokeWidth="1" />
                        {/* Red Delta Line */}
                        <path
                            d="M 0,50 Q 100,55 130,50 T 180,20 L 300,22"
                            fill="none" stroke="#ef4444" strokeWidth="2"
                        />
                        {/* Cursor Point */}
                        <circle cx="180" cy="20" r="4" fill="#ef4444" />
                    </svg>

                    {/* X-Axis Labels */}
                    <div className="absolute left-10 right-0 -bottom-5 flex justify-between text-[10px] text-gray-400 font-mono">
                        <span>00.0</span>
                        <span>25.6</span>
                        <span className="bg-red-500 text-white px-1 rounded">37.93</span>
                        <span>51.2</span>
                        <span>01:16.8</span>
                    </div>
                </div>

                {/* Tabs */}
                <div className="flex justify-center gap-4 mt-8">
                    <button className="text-gray-400 text-xs font-bold">Friction G</button>
                    <button className="text-gray-400 text-xs font-bold">Cornering G</button>
                    <button className="text-gray-400 text-xs font-bold">Combined G</button>
                    <button className="bg-gray-200 text-gray-700 px-3 py-1 rounded-full text-xs font-bold">Time Diff</button>
                </div>

                {/* Footer Toggles */}
                <div className="flex gap-4 mt-6">
                    <button className="flex-1 py-3 rounded-full border border-gray-300 text-gray-600 font-bold text-sm">Summary</button>
                    <button className="flex-1 py-3 rounded-full border border-gray-300 text-gray-600 font-bold text-sm">Single</button>
                </div>
            </div>
        </main>
    );
}
