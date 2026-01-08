import Link from 'next/link';
import { MapPin, Clock, TrendingUp, Wifi } from 'lucide-react';
import MapWrapper from "./components/MapWrapper";

export default function Home() {
  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900">
      {/* Navigation */}
      <nav className="bg-slate-900/50 backdrop-blur-sm border-b border-slate-700">
        <div className="container mx-auto px-6 py-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center space-x-2">
              <div className="w-12 h-12 relative">
                <img
                  src="/logo.png"
                  alt="Much Racing Logo"
                  className="w-full h-full object-contain"
                />
              </div>
              <span className="text-white text-2xl font-bold">Much Racing</span>
            </div>
            <div className="hidden md:flex space-x-6">
              <Link href="/dashboard" className="text-slate-300 hover:text-white transition">Dashboard</Link>
              <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
            </div>
            <Link href="/login" className="bg-orange-500 hover:bg-orange-600 text-white px-6 py-2 rounded-lg transition">
              Login
            </Link>
          </div>
        </div>
      </nav>

      <div className="container mx-auto px-6 py-20">
        <div className="text-center max-w-4xl mx-auto">
          <h1 className="text-5xl md:text-7xl font-bold text-white mb-6">
            Track Your Racing
            <span className="block text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-red-600">
              Performance
            </span>
          </h1>
          <p className="text-xl text-slate-300 mb-8">
            Professional lap timing and telemetry analysis for karting and racing enthusiasts.
            Sync your Much Racing device and unlock detailed performance insights.
          </p>
          <div className="flex flex-col sm:flex-row gap-4 justify-center">
            <Link href="/tracking" className="bg-orange-500 hover:bg-orange-600 text-white px-8 py-4 rounded-lg text-lg font-semibold transition flex items-center justify-center gap-2">
              <Wifi className="w-5 h-5" /> Open Live Satellite
            </Link>
            <Link href="/tracks" className="bg-slate-700 hover:bg-slate-600 text-white px-8 py-4 rounded-lg text-lg font-semibold transition">
              Browse Tracks
            </Link>
          </div>
        </div>

        {/* Live Map Preview Section */}
        <div className="mt-20">
          <div className="text-center mb-8">
            <h2 className="text-3xl font-bold text-white">Live Telemetry Preview</h2>
            <p className="text-slate-400">Real-time data from your Much Racing device.</p>
          </div>
          <div className="w-full h-[600px] bg-slate-800 rounded-2xl overflow-hidden border border-slate-700 relative shadow-2xl shadow-orange-500/10">
            <MapWrapper />
          </div>
        </div>

        {/* Features Grid */}
        <div className="grid md:grid-cols-2 lg:grid-cols-4 gap-6 mt-20">
          <FeatureCard
            icon={<MapPin className="w-8 h-8" />}
            title="Track Database"
            description="Access hundreds of tracks worldwide with GPS-accurate layouts"
          />
          <FeatureCard
            icon={<Clock className="w-8 h-8" />}
            title="Lap Timing"
            description="Precise lap times with sector analysis and best lap tracking"
          />
          <FeatureCard
            icon={<TrendingUp className="w-8 h-8" />}
            title="Performance Analytics"
            description="Detailed telemetry data with speed, RPM, and GPS visualization"
          />
          <FeatureCard
            icon={<Wifi className="w-8 h-8" />}
            title="WiFi Sync"
            description="Seamlessly upload sessions from your Much Racing device"
          />
        </div>

        {/* Stats Section */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-6 mt-20">
          <StatCard number="500+" label="Tracks" />
          <StatCard number="10K+" label="Sessions" />
          <StatCard number="50K+" label="Laps Recorded" />
          <StatCard number="48" label="Countries" />
        </div>
      </div>

      {/* Footer */}
      <footer className="bg-slate-900/50 border-t border-slate-700 mt-20">
        <div className="container mx-auto px-6 py-8">
          <div className="text-center text-slate-400">
            <p>&copy; 2026 Much Racing. Professional Racing Telemetry.</p>
          </div>
        </div>
      </footer>
    </div>
  );
}

function FeatureCard({ icon, title, description }: { icon: React.ReactNode; title: string; description: string }) {
  return (
    <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 hover:border-orange-500 transition">
      <div className="text-orange-500 mb-4">{icon}</div>
      <h3 className="text-white text-xl font-semibold mb-2">{title}</h3>
      <p className="text-slate-400">{description}</p>
    </div>
  );
}

function StatCard({ number, label }: { number: string; label: string }) {
  return (
    <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 text-center">
      <div className="text-3xl md:text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-red-600 mb-2">
        {number}
      </div>
      <div className="text-slate-400">{label}</div>
    </div>
  );
}
