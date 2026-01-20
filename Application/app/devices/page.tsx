'use client';

import {
  ChevronDown,
  ChevronRight,
  Edit2,
  Battery,
  MapPin,
  Signal,
  Gauge,
  Map as MapIcon,
  Zap,
} from 'lucide-react';
import Link from 'next/link';
import { useState } from 'react';


const MOCK_DEVICES = [
  { id: '2230300032', name: '2230300032', model: 'R01', vehicle: 'No Default Vehicle' },
  { id: '1231800177', name: '1231800177', model: 'Mini', vehicle: 'BMW M3' },
  { id: '1231800586', name: '1231800586', model: 'Mini', vehicle: 'Honda Civic' },
  { id: '1221405651', name: '1221405651', model: 'R01', vehicle: 'No Default Vehicle' },
  { id: '2231801121', name: '2231801121', model: 'Mini', vehicle: 'No Default Vehicle' },
  { id: '21423920', name: '21423920', model: 'R01', vehicle: 'No Default Vehicle' },
  { id: 'RaceBox Serres', name: 'RaceBox Serres', model: 'R01', vehicle: 'Track Car' },
  { id: '2230300039', name: '2230300039', model: 'Mini', vehicle: 'No Default Vehicle' },
  { id: '21420568', name: '21420568', model: 'S1', vehicle: 'No Default Vehicle' },
  { id: '3240800033', name: '3240800033', model: 'Mini', vehicle: 'No Default Vehicle' },
];

export default function DevicePage() {
  const [isListOpen, setIsListOpen] = useState(false);
  const [currentDevice, setCurrentDevice] = useState(MOCK_DEVICES[0]);

  const toggleList = () => setIsListOpen(!isListOpen);

  const selectDevice = (device: typeof MOCK_DEVICES[0]) => {
    setCurrentDevice(device);
    setIsListOpen(false);
  };

  return (
    <div className="min-h-screen bg-background text-foreground pb-24">
      {/* Header with Carbon Fiber */}
      <div className="carbon-bg p-4 flex items-center justify-between z-20 relative border-b border-border-color">
        <button onClick={toggleList} className="hover:scale-110 transition-transform">
          <ChevronDown className={`w-8 h-8 text-primary transition-transform duration-300 ${isListOpen ? 'rotate-180' : ''}`} />
        </button>
        <div className="text-center cursor-pointer" onClick={toggleList}>
          <h1 className="text-xl font-racing tracking-wide">{currentDevice.name}</h1>
          <p className="text-text-secondary text-sm font-medium">{currentDevice.vehicle}</p>
        </div>
        <button className="p-2 hover:scale-110 transition-transform">
          <Edit2 className="w-6 h-6 text-primary" />
        </button>
      </div>

      {isListOpen ? (
        // Device List View
        <div className="animate-in fade-in slide-in-from-top-4 duration-200">
          <div className="carbon-bg py-3 px-4 text-center font-racing text-sm text-text-secondary border-b border-border-color">
            CHOOSE DEVICE
          </div>

          <button className="w-full bg-background-secondary p-4 flex items-center justify-between border-b border-border-color hover:bg-primary/10 transition-colors group">
            <span className="font-racing text-foreground group-hover:text-primary transition-colors">ADD NEW DEVICE</span>
            <ChevronRight className="w-5 h-5 text-primary group-hover:translate-x-1 transition-transform" />
          </button>

          <div className="bg-background">
            {MOCK_DEVICES.map((device) => (
              <button
                key={device.id}
                onClick={() => selectDevice(device)}
                className={`w-full p-4 text-center font-data text-base border-b border-border-color transition-all ${currentDevice.id === device.id
                  ? 'bg-primary text-white shadow-[0_0_20px_rgba(220,38,38,0.3)]'
                  : 'text-text-secondary hover:bg-card-bg hover:text-foreground'
                  }`}
              >
                {device.name}
              </button>
            ))}
          </div>
        </div>
      ) : (
        // Single Device Dashboard View
        <div className="px-4 space-y-6 animate-in fade-in zoom-in-95 duration-200">
          {/* Info Strip */}
          <div className="flex items-center justify-between py-4 border-b border-border-color border-t mt-2">
            <span className="text-sm font-data text-text-secondary">S/N: <span className="text-foreground">{currentDevice.id}</span></span>
            <span className="text-sm font-data text-text-secondary">Model: <span className="text-foreground">{currentDevice.model}</span></span>
          </div>

          {/* Status Card - Premium Design */}
          <div className="carbon-bg rounded-xl p-6 shadow-lg border border-border-color relative overflow-hidden">
            {/* Glow Effect */}
            <div className="absolute top-0 right-0 w-32 h-32 bg-highlight/10 rounded-full blur-3xl"></div>

            <div className="flex items-start justify-between mb-4 relative z-10">
              <div className="flex items-center gap-2">
                <Zap className="w-5 h-5 text-highlight animate-pulse" />
                <span className="text-highlight font-racing text-lg drop-shadow-[0_0_8px_rgba(16,185,129,0.4)]">CONNECTED</span>
              </div>

              <div className="flex items-center gap-3 text-text-secondary">
                <Battery className="w-6 h-6 text-highlight" />
                <MapPin className="w-5 h-5 text-foreground" />
                <Signal className="w-5 h-5 text-foreground" />
              </div>
            </div>

            <div className="text-right text-text-secondary font-data text-sm relative z-10">
              <span className="text-foreground font-bold">0.619 m</span> / <span className="text-highlight font-bold">15 sat</span>
            </div>
          </div>

          {/* Action Cards - Enhanced Grid */}
          <div className="grid grid-cols-2 gap-4">
            <Link
              href="/drag"
              className="carbon-bg p-6 rounded-xl border border-border-color hover:border-primary/50 hover:shadow-lg transition-all text-left h-36 flex flex-col justify-between group relative overflow-hidden"
            >
              <div className="absolute inset-0 bg-gradient-to-br from-primary/5 to-transparent opacity-0 group-hover:opacity-100 transition-opacity"></div>
              <Gauge className="w-10 h-10 text-primary group-hover:scale-110 transition-transform relative z-10" />
              <span className="font-racing text-base leading-tight relative z-10 group-hover:text-primary transition-colors">NEW DRAG SESSION</span>
            </Link>

            <Link
              href="/track"
              className="carbon-bg p-6 rounded-xl border border-border-color hover:border-primary/50 hover:shadow-lg transition-all text-left h-36 flex flex-col justify-between group relative overflow-hidden"
            >
              <div className="absolute inset-0 bg-gradient-to-br from-primary/5 to-transparent opacity-0 group-hover:opacity-100 transition-opacity"></div>
              <MapIcon className="w-10 h-10 text-primary group-hover:scale-110 transition-transform relative z-10" />
              <span className="font-racing text-base leading-tight relative z-10 group-hover:text-primary transition-colors">NEW TRACK SESSION</span>
            </Link>
          </div>

          {/* Footer Buttons - Refined */}
          <div className="space-y-3 pt-4">
            <Link
              href="/devices/settings"
              className="block w-full py-4 text-center rounded-xl border border-border-color carbon-bg font-racing text-foreground hover:border-primary/50 hover:text-primary transition-all"
            >
              SETTINGS
            </Link>
            <button className="w-full py-4 text-center rounded-xl border border-primary/30 bg-primary/10 font-racing text-primary hover:bg-primary/20 transition-colors">
              DISCONNECT
            </button>
          </div>
        </div>
      )}

    </div>
  );
}
