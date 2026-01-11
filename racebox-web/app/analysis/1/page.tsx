"use client";

import { ChevronLeft, MoreHorizontal, Info } from "lucide-react";
import { useRouter } from "next/navigation";

export default function AnalysisPage() {
    const router = useRouter();

    return (
        <main className="min-h-screen pb-20 bg-background text-foreground">
            {/* Header */}
            <header className="sticky top-0 z-10 bg-white border-b border-gray-100 flex items-center justify-between px-4 h-16">
                <button onClick={() => router.back()} className="p-1">
                    <ChevronLeft className="w-6 h-6 text-primary" />
                </button>
                <h1 className="text-xl font-bold text-gray-700">0-100 kph, 1ft: ON</h1>
                <MoreHorizontal className="w-6 h-6 text-primary" />
            </header>

            <div className="p-4 space-y-4">
                {/* Main Stats Card */}
                <div className="bg-white rounded-xl shadow-sm border border-gray-100 overflow-hidden text-center">
                    <div className="p-4 border-b border-gray-100">
                        <h2 className="text-4xl font-bold text-foreground">5.19</h2>
                        <div className="text-sm text-gray-500 mt-1">Session 02 | Run 2</div>
                        <div className="text-sm text-gray-400">23.10.2024, 15:18</div>
                    </div>

                    <div className="p-3 border-b border-gray-100">
                        <div className="text-sm text-gray-400">Max G-Force</div>
                        <div className="font-bold text-lg">0.84 g</div>
                    </div>

                    <div className="p-3 border-b border-gray-100">
                        <div className="text-sm text-gray-400">Weather</div>
                        <div className="font-bold text-lg">18.5 °C, 1,028.0 hPa, 20%</div>
                    </div>

                    <div className="p-3 flex justify-center items-center gap-2">
                        <div>
                            <div className="text-sm text-gray-400">Density Altitude</div>
                            <div className="font-bold text-lg">784.9 m</div>
                        </div>
                        <Info className="w-4 h-4 text-gray-300" />
                    </div>
                </div>

                {/* Splits List */}
                <div className="bg-white rounded-xl shadow-sm border border-gray-100 overflow-hidden text-center">
                    {['0-10 kph', '0-20 kph', '0-30 kph', '0-40 kph', '0-50 kph'].map((label, i) => (
                        <div key={label} className="p-3 border-b border-gray-50 last:border-0">
                            <div className="text-sm text-gray-400">{label}</div>
                            <div className="font-bold text-lg text-foreground">{(0.42 + i * 0.70).toFixed(2)}</div>
                        </div>
                    ))}
                </div>

                {/* Graph / Video Placeholder */}
                <div className="bg-black rounded-xl overflow-hidden aspect-video relative">
                    <div className="absolute inset-0 flex items-center justify-center text-white/50">
                        Video/Graph Overlay Placeholder
                    </div>
                    {/* Mock Graph Layout from screenshot */}
                    <div className="absolute bottom-0 left-0 right-0 bg-white/90 h-1/2 p-2">
                        <div className="text-center text-primary font-bold p-1 border border-primary inline-block rounded text-xs mb-1 bg-white">02.60</div>
                        <div className="w-full h-full border-t border-gray-200 flex items-end">
                            <div className="w-full h-[60%] bg-gradient-to-tr from-primary/20 to-primary/5 border-t border-primary relative"></div>
                        </div>
                    </div>
                </div>

                {/* Buttons */}
                <div className="flex gap-4">
                    <button className="flex-1 py-3 rounded-full border border-foreground font-bold text-foreground bg-white">Hide</button>
                    <button className="flex-1 py-3 rounded-full border border-foreground font-bold text-foreground bg-white">Compare</button>
                </div>

            </div>
        </main>
    );
}
