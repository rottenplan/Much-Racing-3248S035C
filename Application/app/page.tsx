"use client";

import { Heart, Menu, Cloud, ChevronDown, Car, Trophy, CheckCircle2 } from "lucide-react";
import Link from "next/link";
import SessionCard from "./components/SessionCard";
import BottomNav from "./components/BottomNav";

export default function Home() {
  return (
    <main className="min-h-screen pb-20 bg-background text-foreground">
      {/* Header with Carbon Fiber */}
      <header className="sticky top-0 z-10 carbon-bg backdrop-blur-md border-b border-border-color flex items-center justify-between px-4 h-16">
        <div className="flex items-center gap-3">
          <h1 className="text-xl font-racing text-foreground tracking-wide">DRAG SESSIONS</h1>
          <Cloud className="w-4 h-4 text-highlight animate-pulse" />
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

        <button className="flex items-center gap-2 text-sm text-text-secondary hover:text-foreground transition-colors">
          Sort by Date <ChevronDown className="w-3 h-3" />
        </button>
      </div>

      {/* Content List */}
      <div className="p-4 space-y-4">

        {/* Featured Session Group - Premium Card */}
        <div className="carbon-bg rounded-xl shadow-lg border border-border-color overflow-hidden hover:border-primary/50 transition-all">
          {/* Header of the Group */}
          <div className="p-4 flex items-center justify-between border-b border-border-color bg-gradient-to-r from-background-secondary to-transparent">
            <div>
              <div className="flex items-center gap-3">
                <h3 className="text-lg font-racing text-foreground">14.11.2024</h3>
                <span className="text-xs px-2 py-1 rounded-md bg-primary/20 text-primary font-bold border border-primary/30">
                  2 RUNS
                </span>
              </div>
              <div className="text-sm text-text-secondary flex items-center gap-2 mt-2">
                <Car className="w-4 h-4 text-primary" />
                <span className="font-medium">BMW M3</span>
                <span className="text-border-color">|</span>
                <span>Best 100-200:</span>
                <span className="text-highlight font-data font-bold">5.45s</span>
              </div>
            </div>
            <ChevronDown className="w-5 h-5 text-text-secondary rotate-180 transition-transform" />
          </div>

          {/* Runs Inside */}
          <div className="divide-y divide-border-color">
            <Link
              href="/session/1"
              className="flex items-center justify-between p-4 bg-card-bg hover:bg-card-bg-hover transition-colors group"
            >
              <div className="flex items-center gap-4">
                <span className="text-primary font-racing text-sm w-10 h-10 flex items-center justify-center bg-primary/10 rounded-lg border border-primary/30">
                  03
                </span>
                <div className="flex flex-col">
                  <span className="text-foreground font-data font-bold text-2xl">5.83</span>
                  <span className="text-text-secondary text-xs font-medium">100-200 kph</span>
                </div>
              </div>
              <div className="text-right">
                <div className="text-sm text-warning font-data">-0.38s</div>
                <div className="flex items-center gap-1 text-highlight text-xs font-medium">
                  <CheckCircle2 className="w-3 h-3" />
                  Valid
                </div>
              </div>
            </Link>

            <Link
              href="/session/1"
              className="flex items-center justify-between p-4 bg-card-bg hover:bg-card-bg-hover transition-colors group relative overflow-hidden"
            >
              {/* Best Run Highlight */}
              <div className="absolute inset-0 bg-gradient-to-r from-highlight/5 to-transparent pointer-events-none"></div>

              <div className="flex items-center gap-4 relative z-10">
                <span className="text-highlight font-racing text-sm w-10 h-10 flex items-center justify-center bg-highlight/10 rounded-lg border border-highlight/30">
                  02
                </span>
                <div className="flex flex-col">
                  <span className="text-highlight font-data font-bold text-2xl drop-shadow-[0_0_8px_rgba(16,185,129,0.4)]">
                    5.45
                  </span>
                  <span className="text-text-secondary text-xs font-medium">100-200 kph</span>
                </div>
              </div>
              <div className="text-right relative z-10">
                <div className="flex items-center gap-1 text-highlight text-sm font-racing mb-1">
                  <Trophy className="w-4 h-4" />
                  BEST
                </div>
                <div className="flex items-center gap-1 text-highlight text-xs font-medium">
                  <CheckCircle2 className="w-3 h-3" />
                  Valid
                </div>
              </div>
            </Link>
          </div>
        </div>

        {/* Standard Sessions with Enhanced Cards */}
        <SessionCard
          date="27.11.2024"
          title="Best 400 m: 15.87"
          subtitle="Honda Civic Type R"
          type="drag"
          href="/session/1"
        />

        <SessionCard
          date="25.11.2024"
          title="Best 100-200 kph: 6.02"
          subtitle="BMW M3"
          type="drag"
          href="/session/1"
        />

        <SessionCard
          date="07.11.2024"
          title="Best 1/4 mile: 40.32"
          subtitle="Volkswagen Golf"
          type="drag"
        />

      </div>

      <BottomNav />
    </main>
  );
}
