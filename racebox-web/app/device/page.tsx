'use client';

import Link from 'next/link';
import { Wifi, Upload, Download, Settings, HardDrive, Cpu, Globe, Gauge, Save } from 'lucide-react';
import { useState } from 'react';

const countries = [
  'Argentina', 'Australia', 'Austria', 'Belgium', 'Brazil', 'Canada', 'Chile', 'China', 
  'Colombia', 'Czech Republic', 'Denmark', 'Finland', 'France', 'Germany', 'Greece', 
  'Hong Kong', 'Hungary', 'India', 'Indonesia', 'Ireland', 'Italy', 'Japan', 'Malaysia',
  'Mexico', 'Netherlands', 'New Zealand', 'Norway', 'Philippines', 'Poland', 'Portugal',
  'Russia', 'Singapore', 'South Africa', 'South Korea', 'Spain', 'Sweden', 'Switzerland',
  'Taiwan', 'Thailand', 'Turkey', 'UAE', 'United Kingdom', 'USA', 'Vietnam'
];

export default function DevicePage() {
  const [selectedCountries, setSelectedCountries] = useState<string[]>(['Indonesia', 'Malaysia', 'Singapore']);
  const [activeEngine, setActiveEngine] = useState('1');
  const [units, setUnits] = useState('kmh');
  const [temperature, setTemperature] = useState('celsius');
  const [gnss, setGnss] = useState('gps');
  const [contrast, setContrast] = useState(50);
  const [powerSave, setPowerSave] = useState(5);

  const toggleCountry = (country: string) => {
    setSelectedCountries(prev =>
      prev.includes(country)
        ? prev.filter(c => c !== country)
        : [...prev, country]
    );
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900">
      {/* Navigation */}
      <nav className="bg-slate-900/50 backdrop-blur-sm border-b border-slate-700">
        <div className="container mx-auto px-6 py-4">
          <div className="flex items-center justify-between">
            <Link href="/" className="flex items-center space-x-2">
              <div className="w-12 h-12 relative">
                <img 
                  src="/logo.png" 
                  alt="Much Racing Logo" 
                  className="w-full h-full object-contain"
                />
              </div>
              <span className="text-white text-2xl font-bold">Much Racing</span>
            </Link>
            <div className="hidden md:flex space-x-6">
              <Link href="/dashboard" className="text-slate-300 hover:text-white transition">Dashboard</Link>
              <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-white font-semibold">Device</Link>
            </div>
          </div>
        </div>
      </nav>

      {/* Content */}
      <div className="container mx-auto px-6 py-12">
        <div className="mb-8">
          <h1 className="text-4xl font-bold text-white mb-2">My Device</h1>
          <p className="text-slate-400">Configure your Much Racing device settings</p>
        </div>

        {/* Device Status */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8 mb-8">
          <div className="flex items-center justify-between mb-6">
            <div>
              <h2 className="text-2xl font-bold text-white mb-2">MuchRacing_e05a1b</h2>
              <p className="text-slate-400">MAC: E0:5A:1B:A2:74:44</p>
              <p className="text-slate-400">Firmware: v2.1.0</p>
            </div>
            <div className="flex items-center space-x-2">
              <div className="w-3 h-3 bg-green-500 rounded-full animate-pulse"></div>
              <span className="text-green-400 font-semibold">Connected</span>
            </div>
          </div>

          {/* Storage */}
          <div className="mb-6">
            <div className="flex items-center justify-between mb-2">
              <span className="text-slate-400 flex items-center">
                <HardDrive className="w-5 h-5 mr-2" />
                Storage Usage
              </span>
              <span className="text-white font-semibold">7 MB / 31,992 MB</span>
            </div>
            <div className="w-full bg-slate-900 rounded-full h-3">
              <div className="bg-gradient-to-r from-orange-500 to-red-600 h-3 rounded-full" style={{ width: '0.02%' }}></div>
            </div>
          </div>

          <div className="grid md:grid-cols-3 gap-6">
            <div className="bg-slate-900/50 border border-slate-700 rounded-lg p-4">
              <Wifi className="w-8 h-8 text-orange-500 mb-2" />
              <div className="text-slate-400 text-sm mb-1">WiFi Status</div>
              <div className="text-white font-semibold">Connected</div>
            </div>
            <div className="bg-slate-900/50 border border-slate-700 rounded-lg p-4">
              <Upload className="w-8 h-8 text-blue-500 mb-2" />
              <div className="text-slate-400 text-sm mb-1">Last Sync</div>
              <div className="text-white font-semibold">2 hours ago</div>
            </div>
            <div className="bg-slate-900/50 border border-slate-700 rounded-lg p-4">
              <Download className="w-8 h-8 text-green-500 mb-2" />
              <div className="text-slate-400 text-sm mb-1">Sessions</div>
              <div className="text-white font-semibold">24 synced</div>
            </div>
          </div>
        </div>

        {/* Device Settings */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8 mb-8">
          <h2 className="text-2xl font-bold text-white mb-6 flex items-center">
            <Settings className="w-6 h-6 mr-2" />
            Device Settings
          </h2>

          <div className="grid md:grid-cols-2 gap-6">
            {/* Units */}
            <div>
              <label className="text-slate-400 text-sm mb-2 block">Speed Units</label>
              <select
                value={units}
                onChange={(e) => setUnits(e.target.value)}
                className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-white focus:outline-none focus:border-orange-500"
              >
                <option value="kmh">Km/h</option>
                <option value="mph">Mph</option>
              </select>
            </div>

            {/* Temperature */}
            <div>
              <label className="text-slate-400 text-sm mb-2 block">Temperature Units</label>
              <select
                value={temperature}
                onChange={(e) => setTemperature(e.target.value)}
                className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-white focus:outline-none focus:border-orange-500"
              >
                <option value="celsius">Celsius</option>
                <option value="fahrenheit">Fahrenheit</option>
              </select>
            </div>

            {/* GNSS */}
            <div>
              <label className="text-slate-400 text-sm mb-2 block">GNSS Mode</label>
              <select
                value={gnss}
                onChange={(e) => setGnss(e.target.value)}
                className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-white focus:outline-none focus:border-orange-500"
              >
                <option value="gps">GPS</option>
                <option value="gps_glonass">GPS + GLONASS</option>
                <option value="gps_galileo">GPS + Galileo</option>
                <option value="all">All (GPS + GLONASS + Galileo)</option>
              </select>
            </div>

            {/* Contrast */}
            <div>
              <label className="text-slate-400 text-sm mb-2 block">Screen Contrast: {contrast}%</label>
              <input
                type="range"
                min="0"
                max="100"
                value={contrast}
                onChange={(e) => setContrast(Number(e.target.value))}
                className="w-full"
              />
            </div>

            {/* Power Save */}
            <div>
              <label className="text-slate-400 text-sm mb-2 block">Power Save (minutes)</label>
              <select
                value={powerSave}
                onChange={(e) => setPowerSave(Number(e.target.value))}
                className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-white focus:outline-none focus:border-orange-500"
              >
                <option value={1}>1 minute</option>
                <option value={5}>5 minutes</option>
                <option value={10}>10 minutes</option>
                <option value={30}>30 minutes</option>
                <option value={0}>Never</option>
              </select>
            </div>
          </div>

          <button className="mt-6 bg-orange-500 hover:bg-orange-600 text-white px-6 py-2 rounded-lg transition flex items-center">
            <Save className="w-4 h-4 mr-2" />
            Save Settings
          </button>
        </div>

        {/* Engine Management */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8 mb-8">
          <h2 className="text-2xl font-bold text-white mb-6 flex items-center">
            <Cpu className="w-6 h-6 mr-2" />
            Engine Management
          </h2>

          <div className="mb-4">
            <label className="text-slate-400 text-sm mb-2 block">Active Engine</label>
            <select
              value={activeEngine}
              onChange={(e) => setActiveEngine(e.target.value)}
              className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-white focus:outline-none focus:border-orange-500"
            >
              <option value="1">Engine 1</option>
              <option value="2">Engine 2</option>
              <option value="3">Engine 3</option>
            </select>
          </div>

          <div className="space-y-4">
            {[1, 2, 3].map((engineNum) => (
              <div key={engineNum} className="bg-slate-900/50 border border-slate-700 rounded-lg p-4">
                <div className="grid md:grid-cols-2 gap-4">
                  <div>
                    <label className="text-slate-400 text-sm mb-2 block">Engine {engineNum} Name</label>
                    <input
                      type="text"
                      defaultValue={engineNum === 1 ? 'Much Racing' : `Engine ${engineNum}`}
                      className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-white focus:outline-none focus:border-orange-500"
                    />
                  </div>
                  <div>
                    <label className="text-slate-400 text-sm mb-2 block flex items-center">
                      <Gauge className="w-4 h-4 mr-1" />
                      Engine Hours
                    </label>
                    <input
                      type="number"
                      defaultValue={engineNum === 1 ? '24.5' : '0.0'}
                      step="0.1"
                      className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-white focus:outline-none focus:border-orange-500"
                    />
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Track Database Selection */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8">
          <h2 className="text-2xl font-bold text-white mb-4 flex items-center">
            <Globe className="w-6 h-6 mr-2" />
            Global Track Database
          </h2>
          <p className="text-slate-400 mb-6">
            Select countries to load track maps onto your device. 
            <span className="text-orange-400"> Note: More countries = longer track detection time.</span>
          </p>

          <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-3">
            {countries.map((country) => (
              <label key={country} className="flex items-center space-x-2 cursor-pointer hover:bg-slate-700/30 p-2 rounded transition">
                <input
                  type="checkbox"
                  checked={selectedCountries.includes(country)}
                  onChange={() => toggleCountry(country)}
                  className="w-4 h-4 text-orange-500 bg-slate-900 border-slate-700 rounded focus:ring-orange-500"
                />
                <span className="text-slate-300 text-sm">{country}</span>
              </label>
            ))}
          </div>

          <div className="mt-6 p-4 bg-slate-900/50 border border-slate-700 rounded-lg">
            <div className="text-slate-400 text-sm mb-2">Selected Countries: {selectedCountries.length}</div>
            <div className="flex flex-wrap gap-2">
              {selectedCountries.map((country) => (
                <span key={country} className="bg-orange-500/20 text-orange-400 px-3 py-1 rounded-lg text-sm">
                  {country}
                </span>
              ))}
            </div>
          </div>

          <button className="mt-6 bg-orange-500 hover:bg-orange-600 text-white px-6 py-2 rounded-lg transition flex items-center">
            <Save className="w-4 h-4 mr-2" />
            Save Track Selection
          </button>
        </div>
      </div>
    </div>
  );
}
