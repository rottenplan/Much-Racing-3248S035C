"use client";

import { Heart, Menu, Trophy, CheckCircle2, ChevronDown } from "lucide-react";
import Link from "next/link";
import SessionCard from "../components/SessionCard";
import BottomNav from "../components/BottomNav";

export default function TrackPage() {
    return (
        <main className="min-h-screen pb-20 bg-background text-foreground">
            {/* Header with Carbon Fiber */}
            <header className="sticky top-0 z-10 carbon-bg backdrop-blur-md border-b border-border-color flex items-center justify-between px-4 h-16">
                <div className="flex items-center gap-3">
                    <h1 className="text-xl font-racing tracking-wide">TRACK SESSIONS</h1>
                </div>
                <div className="flex gap-4">
                    <Heart className="w-6 h-6 text-primary hover:scale-110 transition-transform cursor-pointer" />
                    <Menu className="w-6 h-6 text-primary hover:scale-110 transition-transform cursor-pointer" />
                </div>
            </header>

            {/* Filter Bar */}
            <div className="px-4 py-3 bg-background-secondary flex items-center justify-between border-b border-border-color">
                <button className="flex items-center gap-2 px-4 py-2 rounded-lg border border-primary text-sm font-medium text-foreground bg-primary/10 hover:bg-primary/20 transition-colors">
                    <CheckCircle2 className="w-4 h-4 text-primary" />
                    Only Mine
                </button>
            </div>

            {/* Content List */}
            <div className="p-4 space-y-4">

                {/* Featured Session Group - Premium Card */}
                <div className="carbon-bg rounded-xl shadow-lg border border-border-color overflow-hidden hover:border-primary/50 transition-all">
                    {/* Header */}
                    <div className="p-4 flex items-center justify-between border-b border-border-color bg-gradient-to-r from-background-secondary to-transparent">
                        <div>
                            <div className="flex items-center gap-3">
                                <h3 className="text-lg font-racing text-foreground">23.11.2024</h3>
                                <span className="text-xs px-2 py-1 rounded-md bg-primary/20 text-primary font-bold border border-primary/30">
                                    2 LAPS
                                </span>
                            </div>
                            <div className="text-sm text-text-secondary flex items-center gap-2 mt-2">
                                <span className="font-medium">Serres Automotive</span>
                                <span className="text-border-color">|</span>
                                <span>Best:</span>
                                <span className="text-highlight font-data font-bold">01:24.71</span>
                            </div>
                        </div>
                        <ChevronDown className="w-5 h-5 text-text-secondary rotate-180 transition-transform" />
                    </div>

                    {/* Laps Inside */}
                    <div className="divide-y divide-border-color">
                        <Link
                            href="/session/1"
                            className="flex items-center justify-between p-4 bg-card-bg hover:bg-card-bg-hover transition-colors group"
                        >
                            <div className="flex items-center gap-4">
                                <span className="text-highlight font-racing text-sm w-10 h-10 flex items-center justify-center bg-highlight/10 rounded-lg border border-highlight/30">
                                    04
                                </span>
                                <div className="flex flex-col">
                                    <span className="text-text-secondary text-xs font-medium">13:36</span>
                                    <span className="text-highlight text-sm font-medium">15 Laps</span>
                                </div>
                            </div>
                            <div className="text-right">
                                <span className="text-highlight font-data font-bold text-2xl drop-shadow-[0_0_8px_rgba(16,185,129,0.4)]">
                                    01:24.71
                                </span>
                            </div>
                        </Link>

                        <Link
                            href="/session/1"
                            className="flex items-center justify-between p-4 bg-card-bg hover:bg-card-bg-hover transition-colors group"
                        >
                            <div className="flex items-center gap-4">
                                <span className="text-primary font-racing text-sm w-10 h-10 flex items-center justify-center bg-primary/10 rounded-lg border border-primary/30">
                                    01
                                </span>
                                <div className="flex flex-col">
                                    <span className="text-text-secondary text-xs font-medium">13:02</span>
                                    <span className="text-text-secondary text-sm font-medium">14 Laps</span>
                                </div>
                            </div>
                            <div className="text-right">
                                <span className="text-foreground font-data font-bold text-2xl">
                                    01:25.42
                                </span>
                            </div>
                        </Link>
                    </div>
                </div>

                {/* Standard Sessions with Enhanced Cards */}
                <SessionCard
                    date="22.11.2024"
                    title="Serres Automotive"
                    subtitle="Best Lap: 01:22.84"
                    highlight="01:22.84"
                    highlightColor="green"
                    type="track"
                    href="/session/1"
                />

                <SessionCard
                    date="09.11.2024"
                    title="Serres Automotive"
                    subtitle="Best Lap: 01:24.31"
                    highlight="01:24.31"
                    highlightColor="normal"
                    type="track"
                    href="/session/1"
                />

            </div>

            <BottomNav />
        </main>
    );
}
