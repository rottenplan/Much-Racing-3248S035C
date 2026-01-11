"use client";

import { Heart, Menu, X } from "lucide-react";
import Link from "next/link";
import SessionCard from "./components/SessionCard";

export default function Home() {
  return (
    <main className="min-h-screen pb-20 bg-background text-foreground">
      {/* Header */}
      <header className="sticky top-0 z-10 bg-white border-b border-gray-100 flex items-center justify-between px-4 h-16">
        <h1 className="text-xl font-bold text-gray-700">Drag Sessions</h1>
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

        {/* Expanded Style Session Group */}
        <div className="bg-card-bg rounded-xl shadow-sm border border-gray-100 overflow-hidden">
          {/* Header of the Group */}
          <div className="p-4 flex items-center justify-between border-b border-gray-100">
            <div>
              <h3 className="text-lg font-bold text-foreground">14.11.2024, 2 Sessions</h3>
              <div className="text-sm text-gray-600">Best 100-200 kph: 5.45</div>
            </div>
            <div className="text-gray-400">^</div>
          </div>

          {/* Runs Inside */}
          <div className="p-0">
            <Link href="/session/1" className="flex items-center justify-between p-3 border-b border-gray-50 bg-white hover:bg-gray-50">
              <span className="text-red-500 font-bold w-8">03</span>
              <span className="text-gray-600 text-sm">1 Run</span>
              <span className="text-primary font-medium">100-200 kph</span>
              <span className="text-primary font-bold text-lg">5.83 &gt;</span>
            </Link>
            <Link href="/session/1" className="flex items-center justify-between p-3 bg-white hover:bg-gray-50">
              <span className="text-highlight font-bold w-8">02</span>
              <span className="text-gray-600 text-sm">1 Run</span>
              <span className="text-highlight font-medium">100-200 kph</span>
              <span className="text-highlight font-bold text-lg">5.45 &gt;</span>
            </Link>
          </div>
        </div>

        {/* Standard Sessions */}
        <SessionCard
          date="27.11.2024, 3 Sessions"
          title="Best 400 m: 15.87"
          subtitle=""
          type="drag"
          href="/session/1"
        />

        <SessionCard
          date="25.11.2024, 1 Session"
          title="Best 100-200 kph: 6.02"
          subtitle=""
          type="drag"
          href="/session/1"
        />

        <SessionCard
          date="07.11.2024, 1 Session"
          title="Best 1/4 mile: 40.32"
          subtitle=""
          type="drag"
        />

        <SessionCard
          date="25.10.2024, 1 Session"
          title="Best 100 kph 2 m: 0.06"
          subtitle=""
          type="drag"
        />

      </div>
    </main>
  );
}
