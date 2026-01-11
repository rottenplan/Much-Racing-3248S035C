"use client";

import { Heart, Menu, X } from "lucide-react";
import Link from "next/link";
import SessionCard from "../components/SessionCard";

export default function TrackPage() {
    return (
        <main className="min-h-screen pb-20 bg-background text-foreground">
            {/* Header */}
            <header className="sticky top-0 z-10 bg-white border-b border-gray-100 flex items-center justify-between px-4 h-16">
                <h1 className="text-xl font-bold text-gray-700">Track Sessions</h1>
                <div className="flex gap-4">
                    <Heart className="w-6 h-6 text-primary" />
                    <Menu className="w-6 h-6 text-primary" />
                </div>
            </header>

            {/* Filter Bar */}
            <div className="px-4 py-3 bg-white">
                <button className="flex items-center gap-2 px-3 py-1.5 rounded-full border border-primary text-sm font-medium text-gray-700">
                    Only Mine <X className="w-4 h-4 text-primary" />
                </button>
            </div>

            {/* Content List */}
            <div className="p-4 space-y-4">

                {/* Expanded Group Example */}
                <div className="bg-card-bg rounded-xl shadow-sm border border-gray-100 overflow-hidden">
                    {/* Header */}
                    <div className="p-4 border-b border-gray-100 flex justify-between">
                        <div>
                            <h3 className="text-lg font-bold text-foreground">23.11.2024</h3>
                            <div className="text-sm text-gray-600">Serres Automotive, 01:24.71</div>
                        </div>
                        <div className="text-gray-400">^</div>
                    </div>

                    {/* Rows */}
                    <div className="p-0">
                        <Link href="/session/1" className="flex items-center justify-between p-3 border-b border-gray-50 bg-white hover:bg-gray-50">
                            <span className="text-highlight font-medium w-8">04</span>
                            <span className="text-gray-500 font-bold text-sm">13:36</span>
                            <span className="text-highlight font-medium">15 Laps</span>
                            <span className="text-gray-800 font-bold text-lg">01:24.71 &gt;</span>
                        </Link>
                        <Link href="/session/1" className="flex items-center justify-between p-3 bg-white hover:bg-gray-50">
                            <span className="text-gray-400 font-medium w-8">01</span>
                            <span className="text-gray-500 font-bold text-sm">13:02</span>
                            <span className="text-gray-600 font-medium">14 Laps</span>
                            <span className="text-gray-800 font-bold text-lg">01:25.42 &gt;</span>
                        </Link>
                    </div>
                </div>

                <SessionCard
                    date="22.11.2024"
                    title="Serres Automotive"
                    subtitle="01:22.84"
                    highlight=">"
                    type="track"
                    href="/session/1"
                />

                <SessionCard
                    date="09.11.2024"
                    title="Serres Automotive"
                    subtitle="01:24.31"
                    highlight=">"
                    type="track"
                    href="/session/1"
                />

            </div>
        </main>
    );
}
