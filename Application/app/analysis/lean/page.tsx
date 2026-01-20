"use client";

import {
    ChevronLeft,
    MoreHorizontal,
    Layers,
    ChevronRight
} from "lucide-react";
import Link from "next/link";
import { useRouter } from "next/navigation";

export default function LeanAnalysisPage() {
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

            {/* Map Area */}
            <div className="flex-1 relative bg-gray-50 overflow-hidden min-h-[300px]">
                {/* Map Grid */}
                <div className="absolute inset-0 grid grid-cols-[repeat(20,1fr)] grid-rows-[repeat(20,1fr)] opacity-10 pointer-events-none">
                    {[...Array(400)].map((_, i) => <div key={i} className="border border-gray-300"></div>)}
                </div>

                {/* Mock Map SVG */}
                <svg className="absolute inset-0 w-full h-full" viewBox="0 0 400 500" preserveAspectRatio="xMidYMid slice">
                    {/* Track Surface */}
                    <path
                        d="M 50,150 C 50,150 150,200 180,250 S 200,400 150,500 L 300,450 C 350,400 350,300 300,250 L 200,100 Z"
                        fill="none" stroke="#e5e7eb" strokeWidth="50" strokeLinecap="round" strokeLinejoin="round"
                        transform="scale(0.8) translate(50, 50)"
                    />

                    {/* Lean Path (Color coded? Mock with blue) */}
                    <path
                        d="M 50,150 C 50,150 150,200 180,250 S 200,400 150,500 L 300,450 C 350,400 350,300 300,250 L 200,100 Z"
                        fill="none" stroke="#60a5fa" strokeWidth="4" strokeLinecap="round" strokeLinejoin="round"
                        transform="scale(0.8) translate(50, 50)"
                        className="opacity-80"
                    />

                    {/* Cursor Dot */}
                    <circle cx="180" cy="280" r="10" fill="#60a5fa" stroke="white" strokeWidth="3" className="shadow-lg" />
                </svg>

                <div className="absolute top-4 right-4">
                    <Layers className="w-8 h-8 text-red-600 bg-white rounded-full p-1.5 shadow-md" />
                </div>
            </div>

            {/* Graph Sheet */}
            <div className="bg-white rounded-t-3xl shadow-[0_-5px_20px_rgba(0,0,0,0.1)] p-4 pb-8 z-20 relative -mt-6">

                {/* Cursor Label */}
                <div className="absolute -top-3 left-1/2 -translate-x-1/2">
                    <div className="bg-white px-2 rounded-full shadow-sm">
                        <span className="bg-blue-50 text-blue-600 text-xs font-bold px-2 py-0.5 rounded border border-blue-200">
                            R 51
                        </span>
                    </div>
                    <div className="w-[1px] h-36 bg-blue-400 absolute left-1/2 top-4"></div>
                </div>

                {/* Graph Area */}
                <div className="h-44 w-full relative mt-4 border-b border-gray-200">
                    {/* Y-Axis Labels */}
                    <div className="absolute left-0 top-0 bottom-0 flex flex-col justify-between text-[10px] text-blue-600 font-bold font-mono py-2">
                        <span>R 40</span>
                        <span>R 20</span>
                        <span className="text-gray-400">0</span>
                        <span>L 20</span>
                        <span>L 40</span>
                    </div>

                    {/* Mock Graph Line */}
                    <svg className="absolute ml-10 inset-0 w-[calc(100%-40px)] h-full overflow-visible" preserveAspectRatio="none">
                        <line x1="0" y1="50%" x2="100%" y2="50%" stroke="#e5e7eb" strokeWidth="1" />

                        {/* Lean Line Mock */}
                        <path
                            d="M 0,50 Q 20,10 40,50 T 80,90 T 130,50 T 180,10 T 230,50 T 280,80 L 300,50"
                            fill="none" stroke="#2563eb" strokeWidth="2"
                        />
                        {/* Secondary Lines (Speed/Accel shadows) */}
                        <path
                            d="M 0,60 Q 30,50 60,60 T 120,40 T 180,60 L 300,55"
                            fill="none" stroke="#fca5a5" strokeWidth="1" opacity="0.5"
                        />
                        <path
                            d="M 0,40 Q 40,45 80,40 T 160,35 T 240,40 L 300,45"
                            fill="none" stroke="#86efac" strokeWidth="1" opacity="0.5"
                        />
                    </svg>

                    {/* X-Axis Labels */}
                    <div className="absolute left-10 right-0 -bottom-5 flex justify-between text-[10px] text-gray-400 font-mono">
                        <span>25.6</span>
                        <span className="bg-red-500 text-white px-1 rounded">56.22</span>
                        <span>01:16.8</span>
                    </div>
                </div>

                {/* Tabs/Legend */}
                <div className="flex justify-center flex-wrap gap-4 mt-8">
                    <LegendItem label="Speed (kph)" color="bg-gray-400" />
                    <LegendItem label="Acceleration G" color="bg-green-500" />
                    <LegendItem label="Lean Angle" color="bg-blue-600" active />
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
