"use client";

import { ChevronLeft, MoreHorizontal, ChevronRight, PlayCircle } from "lucide-react";
import Link from "next/link";
import { useRouter } from "next/navigation";

export default function SessionPage() {
    const router = useRouter();

    return (
        <main className="min-h-screen pb-20 bg-background text-foreground">
            {/* Header */}
            <header className="sticky top-0 z-10 bg-white border-b border-gray-100 flex items-center justify-between px-4 h-16">
                <button onClick={() => router.back()} className="p-1">
                    <ChevronLeft className="w-6 h-6 text-primary" />
                </button>
                <h1 className="text-xl font-bold text-gray-700">Session 02</h1>
                <MoreHorizontal className="w-6 h-6 text-primary" />
            </header>

            <div className="p-4 space-y-4">
                {/* Info Card */}
                <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-4">
                    <div className="flex justify-between items-start mb-2">
                        <div>
                            <div className="font-bold text-foreground">23.10.2024, 15:12</div>
                            <div className="text-sm text-gray-500">2 Runs</div>
                        </div>
                        <div className="text-right">
                            <div className="text-sm text-gray-400">1FT Rollout</div>
                            <div className="font-bold text-foreground">ON</div>
                        </div>
                    </div>
                    <div className="border-t border-gray-100 my-2 pt-2">
                        <div className="text-sm text-gray-400">Shared by Alexander Kolarov</div>
                    </div>
                    <div className="border-t border-gray-100 my-2 pt-2">
                        <div className="font-bold">Audi RS3</div>
                    </div>
                    <div className="border-t border-gray-100 mt-2 pt-2 flex justify-between items-center">
                        <span className="font-bold text-foreground">10 videos</span>
                        <span className="text-primary font-bold cursor-pointer">View</span>
                    </div>
                </div>

                {/* Notes Input */}
                <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-3">
                    <input type="text" placeholder="No notes" className="w-full outline-none text-gray-600" />
                </div>

                {/* Tab Switch */}
                <div className="flex justify-end gap-4 text-sm font-bold">
                    <span className="text-gray-400">Show:</span>
                    <span className="text-gray-400 cursor-pointer">All</span>
                    <span className="text-primary cursor-pointer border-b-2 border-primary pb-0.5">Best</span>
                </div>

                {/* Results List */}
                <div className="bg-white rounded-xl shadow-sm border border-gray-100 overflow-hidden">
                    <div className="p-4 bg-gray-50 border-b border-gray-100 text-center">
                        <h3 className="text-xl font-bold text-gray-700">Best Results</h3>
                    </div>

                    <ResultRow label="60 ft" value="2.60" sub="Terminal Speed: 50.17 kph" />
                    <ResultRow label="1/4 mile" value="13.18" sub="Terminal Speed: 184.49 kph" />
                    <Link href="/analysis/1">
                        <ResultRow label="0-100 kph" value="5.19" sub="Distance: 75.0 m" />
                    </Link>
                    <ResultRow label="100-200 kph" value="10.42" sub="Distance: 460.0 m" />
                </div>

            </div>
        </main>
    );
}

function ResultRow({ label, value, sub }: { label: string, value: string, sub: string }) {
    return (
        <div className="p-4 border-b border-gray-50 last:border-0 hover:bg-gray-50 cursor-pointer">
            <div className="flex justify-between items-center mb-1">
                <span className="text-highlight font-bold text-lg">{label}</span>
                <div className="flex items-center gap-2">
                    <span className="text-highlight font-bold text-lg">{value}</span>
                    <ChevronRight className="w-4 h-4 text-gray-400" />
                </div>
            </div>
            <div className="text-sm text-gray-500">{sub}</div>
        </div>
    )
}
