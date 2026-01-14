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
      {/* Header */}
      <div className="p-4 flex items-center justify-between z-20 relative bg-background">
        <button onClick={toggleList}>
          <ChevronDown className={`w-8 h-8 text-primary transition-transform ${isListOpen ? 'rotate-180' : ''}`} />
        </button>
        <div className="text-center cursor-pointer" onClick={toggleList}>
          <h1 className="text-xl font-bold">{currentDevice.name}</h1>
          <p className="text-text-secondary text-sm">{currentDevice.vehicle}</p>
        </div>
        <button className="p-2">
          <Edit2 className="w-6 h-6 text-primary" />
        </button>
      </div>

      {isListOpen ? (
        // Device List View
        <div className="animate-in fade-in slide-in-from-top-4 duration-200">
          <div className="bg-card-bg py-2 px-4 text-center font-medium text-text-secondary border-b border-border-color">
            Choose device
          </div>

          <button className="w-full bg-background p-4 flex items-center justify-between border-b border-border-color">
            <span className="font-bold text-foreground">Add New Device</span>
            <ChevronRight className="w-5 h-5 text-text-secondary" />
          </button>

          <div className="bg-background">
            {MOCK_DEVICES.map((device) => (
              <button
                key={device.id}
                onClick={() => selectDevice(device)}
                className={`w-full p-4 text-center font-bold text-lg border-b border-border-color transition-colors ${currentDevice.id === device.id
                  ? 'bg-primary text-white'
                  : 'text-text-secondary hover:bg-card-bg'
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
            <span className="text-lg font-medium text-text-secondary">S/N: {currentDevice.id}</span>
            <span className="text-lg font-medium text-text-secondary">Model: {currentDevice.model}</span>
          </div>

          {/* Status Card */}
          <div className="bg-card-bg rounded-xl p-6 shadow-sm border border-border-color">
            <div className="flex items-start justify-between mb-4">
              <span className="text-highlight font-bold text-lg">Connected</span>

              <div className="flex items-center gap-3 text-text-secondary">
                <Battery className="w-6 h-6 text-highlight" />
                <MapPin className="w-5 h-5" />
                <Signal className="w-5 h-5" />
              </div>
            </div>

            <div className="text-right text-text-secondary font-mono">
              0.619 m / 15 sat
            </div>
          </div>

          {/* Action Cards */}
          <div className="grid grid-cols-2 gap-4">
            <Link href="/drag" className="bg-card-bg p-4 rounded-xl border border-border-color hover:bg-white/5 transition-colors text-left h-32 flex flex-col justify-between group">
              <div className="flex justify-between items-start w-full">
                <Gauge className="w-8 h-8 text-foreground group-hover:text-primary transition-colors" />
              </div>
              <span className="font-bold text-lg leading-tight">New Drag Session</span>
            </Link>

            <button className="bg-card-bg p-4 rounded-xl border border-border-color hover:bg-white/5 transition-colors text-left h-32 flex flex-col justify-between group">
              <div className="flex justify-between items-start w-full">
                <div className="flex-1"></div>
                <MapIcon className="w-8 h-8 text-foreground group-hover:text-primary transition-colors" />
              </div>
              <span className="font-bold text-lg leading-tight">New Track Session</span>
            </button>
          </div>

          {/* Footer Buttons */}
          <div className="space-y-3 pt-4">
            <Link href="/devices/settings" className="block w-full py-4 text-center rounded-full border border-border-color bg-card-bg font-bold text-foreground hover:bg-white/5 transition-colors">
              Settings
            </Link>
            <button className="w-full py-4 text-center rounded-full border border-border-color bg-card-bg font-bold text-foreground hover:bg-white/5 transition-colors">
              Disconnect
            </button>
          </div>
        </div>
      )}
    </div>
  );
}
