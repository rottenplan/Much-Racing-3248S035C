"use client";

import { Heart, Menu, X, Cloud, ChevronDown, Car } from "lucide-react";
import Link from "next/link";
import SessionCard from "./components/SessionCard";

export default function Home() {
  return (
    <main className="min-h-screen pb-20 bg-background text-foreground">
      {/* Header */}
      <header className="sticky top-0 z-10 bg-background/80 backdrop-blur-md border-b border-border-color flex items-center justify-between px-4 h-16">
        <div className="flex items-center gap-2">
          <h1 className="text-xl font-bold text-foreground">Drag Sessions</h1>
          <Cloud className="w-4 h-4 text-green-500" />
        </div>
        <div className="flex gap-4">
          <Heart className="w-6 h-6 text-primary" />
          <Menu className="w-6 h-6 text-primary" />
        </div>
      </header>

      {/* Filter Bar */}
      <div className="px-4 py-3 bg-background flex items-center justify-between">
        <button className="flex items-center gap-2 px-3 py-1.5 rounded-full border border-primary text-sm font-medium text-text-secondary bg-card-bg">
          Only Mine <X className="w-4 h-4 text-primary" />
        </button>

        <button className="flex items-center gap-2 text-sm text-text-secondary">
          Sort by Date <ChevronDown className="w-3 h-3" />
        </button>
      </div>

      {/* Content List */}
      <div className="p-4 space-y-4">

        {/* Expanded Style Session Group */}
        <div className="bg-card-bg rounded-xl shadow-sm border border-border-color overflow-hidden">
          {/* Header of the Group */}
          <div className="p-4 flex items-center justify-between border-b border-border-color bg-zinc-900/50">
            <div>
              <div className="flex items-center gap-2">
                <h3 className="text-lg font-bold text-foreground">14.11.2024</h3>
                <span className="text-xs px-2 py-0.5 rounded bg-white/10 text-text-secondary">2 Runs</span>
              </div>
              <div className="text-sm text-text-secondary flex items-center gap-2 mt-1">
                <Car className="w-3 h-3" /> BMW M3
                <span className="text-zinc-600">|</span>
                Best 100-200: <span className="text-highlight font-bold">5.45</span>
              </div>
            </div>
            <div className="text-zinc-600 rotate-180 transform">^</div>
          </div>

          {/* Runs Inside */}
          <div className="p-0">
            <Link href="/session/1" className="flex items-center justify-between p-3 border-b border-border-color bg-card-bg hover:bg-white/5 group">
              <span className="text-red-500 font-bold w-8 text-center bg-red-500/10 rounded py-1 text-xs">03</span>
              <div className="flex flex-col">
                <span className="text-primary font-bold text-lg">5.83</span>
                <span className="text-text-secondary text-xs">100-200 kph</span>
              </div>
              <div className="text-right">
                <div className="text-xs text-zinc-500">-0.38s</div>
                <div className="text-zinc-600 text-xs">Valid</div>
              </div>
            </Link>
            <Link href="/session/1" className="flex items-center justify-between p-3 bg-card-bg hover:bg-white/5">
              <span className="text-green-500 font-bold w-8 text-center bg-green-500/10 rounded py-1 text-xs">02</span>
              <div className="flex flex-col">
                <span className="text-highlight font-bold text-lg">5.45</span>
                <span className="text-text-secondary text-xs">100-200 kph</span>
              </div>
              <div className="text-right">
                <div className="text-xs text-highlight font-bold">BEST</div>
                <div className="text-zinc-600 text-xs">Valid</div>
              </div>
            </Link>
          </div>
        </div>

        {/* Standard Sessions */}
        <SessionCard
          date="27.11.2024"
          title="Best 400 m: 15.87"
          subtitle="Honda Civic Type R"
          type="drag"
          href="/session/1"
        />

        <SessionCard
          date="25.11.2024"
          title="Best 100-200 kph: 6.02" // Removed spaces to fit logic
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
    </main>
  );
}
