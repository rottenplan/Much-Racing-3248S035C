'use client';

import Link from 'next/link';
import { useState, useEffect } from 'react';
import { ChevronDown, ChevronRight, ChevronLeft, Search, Plus, Trash2, Save, MapPin, Filter, BarChart2, Smartphone, Info, LogOut } from 'lucide-react';
import TrackCreatorMapWrapper from '../../components/TrackCreatorMapWrapper';

interface TrackPoint {
  lat: number;
  lng: number;
}

export default function CreateTrack() {
  const [trackPoints, setTrackPoints] = useState<TrackPoint[]>([]);
  const [trackName, setTrackName] = useState('Sirkuit Sentul Internasional Karting Cirkuit'); // Default from image
  const [trackLocation, setTrackLocation] = useState('16810 BOGOR - Indonesia');
  const [activeTab, setActiveTab] = useState<'my_tracks' | 'countries'>('my_tracks');
  const [expandedCountries, setExpandedCountries] = useState<string[]>([]);
  const [isDropdownOpen, setIsDropdownOpen] = useState(false);

  // Sector Logic
  const [sectorCount, setSectorCount] = useState(0);

  const handlePointAdd = (lat: number, lng: number) => {
    setTrackPoints([...trackPoints, { lat, lng }]);
  };

  const countries = [
    { name: 'Argentina', count: 28, code: 'AR' },
    { name: 'Australia', count: 133, code: 'AU' },
    { name: 'Austria', count: 13, code: 'AT' },
    { name: 'Bahrain', count: 4, code: 'BH' },
    { name: 'Belarus', count: 3, code: 'BY' },
    { name: 'Belgium', count: 13, code: 'BE' },
    { name: 'Bolivia', count: 15, code: 'BO' },
    { name: 'Brazil', count: 129, code: 'BR' },
    { name: 'Bulgaria', count: 7, code: 'BG' },
    { name: 'Canada', count: 42, code: 'CA' },
    { name: 'Chile', count: 11, code: 'CL' },
    { name: 'China', count: 2, code: 'CN' },
    { name: 'Colombia', count: 15, code: 'CO' },
  ];

  return (
    <div className="min-h-screen bg-background text-foreground flex flex-col font-sans">
      {/* Editor Header */}
      <header className="carbon-bg h-14 flex items-center px-4 border-b border-border-color justify-between relative z-50">
        <div className="flex items-center gap-4">
          <Link href="/tracks" className="text-text-secondary hover:text-foreground transition">
            <ChevronLeft className="w-6 h-6" />
          </Link>
          <div className="flex items-center gap-2 text-sm">
            <MapPin size={16} className="text-primary" />
            <span className="font-semibold font-racing tracking-wide">{trackName}</span>
            <span className="text-text-secondary text-xs">- {trackLocation}</span>
          </div>
        </div>

        <div className="flex items-center gap-3">
          <button className="bg-background-secondary hover:bg-card-bg text-foreground border border-border-color px-3 py-1.5 rounded text-xs font-racing transition flex items-center gap-2">
            <Save className="w-3 h-3" />
            SAVE DRAFT
          </button>
          <button className="bg-primary hover:bg-primary-hover text-white px-3 py-1.5 rounded text-xs font-racing transition shadow-md">
            PUBLISH TRACK
          </button>
        </div>
      </header>

      {/* Main Content Area */}
      <div className="flex flex-1 overflow-hidden">

        {/* Left Sidebar */}
        <div className="w-80 bg-[#1e1e1e] border-r border-[#333] flex flex-col">
          {/* Filter Search */}
          <div className="p-3 border-b border-[#333]">
            <div className="flex gap-2">
              <div className="relative flex-1">
                <input
                  type="text"
                  placeholder=""
                  className="w-full bg-white text-black text-xs px-2 py-1 rounded-sm focus:outline-none"
                />
              </div>
              <button className="bg-[#4caf50] hover:bg-[#45a049] text-white text-xs px-3 py-1 rounded-sm flex items-center gap-1">
                Filter
              </button>
            </div>
          </div>

          {/* My Own Tracks Section */}
          <div className="overflow-y-auto flex-1 custom-scrollbar">
            <div className="mb-1">
              <button
                className="w-full bg-white text-black px-4 py-2 flex items-center justify-between text-xs font-bold border-l-4 border-orange-500"
                onClick={() => setActiveTab('my_tracks')}
              >
                <div className="flex items-center gap-2">
                  <span className="text-orange-500 text-xs">●</span>
                  My Own tracks (3)
                </div>
              </button>
              {activeTab === 'my_tracks' && (
                <div className="bg-[#2d2d2d] py-1">
                  <div className="px-8 py-1 text-xs text-gray-400 hover:text-white cursor-pointer truncate">
                    1. Sirkuit Sentul Internasional Karting Cirkuit
                  </div>
                  <div className="px-8 py-1 text-xs text-gray-400 hover:text-white cursor-pointer truncate">
                    2. Cisarua
                  </div>
                  <div className="px-8 py-1 text-xs text-[#0099cc] hover:text-[#33bbff] cursor-pointer truncate">
                    3. Sentul Internasional Karting Cirkuit
                  </div>
                </div>
              )}
            </div>

            {/* Countries List */}
            {countries.map((c) => (
              <div key={c.name} className="mb-[1px]">
                <button className="w-full bg-[#1e1e1e] hover:bg-[#2d2d2d] text-gray-300 px-4 py-2 flex items-center gap-3 text-xs border-l-4 border-transparent hover:border-gray-500 transition-colors">
                  {/* Flag Placeholder (using emoji for simplicity) */}
                  <span className="w-4 text-center">🏁</span>
                  <span>{c.name} ({c.count})</span>
                </button>
              </div>
            ))}
          </div>

          {/* Sidebar Footer/Tools */}
          <div className="p-2 border-t border-[#333] flex flex-col gap-2">
            {/* Tools icons sidebar */}
          </div>
        </div>

        {/* Map Area (Center) */}
        <div className="flex-1 relative bg-slate-800 flex flex-col">
          <div className="flex-1 relative">
            <TrackCreatorMapWrapper
              points={trackPoints}
              onPointAdd={handlePointAdd}
            />

            {/* Map Controls Overlay (Right Side) */}
            <div className="absolute top-4 right-4 flex flex-col gap-1">
              <button className="bg-white p-1 rounded-sm shadow text-black hover:bg-gray-100"><span className="text-xs font-bold">⛶</span></button>
              <button className="bg-white p-1 rounded-sm shadow text-black hover:bg-gray-100"><span className="text-xs font-bold">+</span></button>
              <button className="bg-white p-1 rounded-sm shadow text-black hover:bg-gray-100"><span className="text-xs font-bold">-</span></button>
            </div>

            {/* Map Controls Bottom Overlay */}
            <div className="absolute bottom-4 left-1/2 transform -translate-x-1/2 flex bg-[#1e1e1e] rounded shadow border border-[#333]">
              <button className="px-3 py-1 text-xs text-white border-r border-[#333] flex items-center gap-1 hover:bg-[#333]">
                <span>🚫</span> No sectors
              </button>
              <button className="px-3 py-1 text-xs text-white border-r border-[#333] hover:bg-[#333]">Delete</button>
              <button className="px-3 py-1 text-xs text-white bg-[#4caf50] hover:bg-[#45a049] border-r border-[#4caf50]">Default</button>
              <button className="px-3 py-1 text-xs text-white border-r border-[#333] hover:bg-[#333]">Delete track</button>
              <button className="px-3 py-1 text-xs text-white hover:bg-[#333]">New</button>
            </div>

          </div>

          {/* Bottom Panel (Timeline / Sectors) */}
          <div className="h-32 bg-white border-t border-gray-300 p-4">
            <div className="flex items-center gap-4 mb-2">
              <span className="text-gray-600 font-bold text-lg">Sectors:</span>
              <div className="flex gap-1">
                <button className="bg-[#0099cc] hover:bg-[#0088bb] text-white p-1 rounded-sm w-6 h-6 flex items-center justify-center">
                  <ChevronLeft size={14} />
                </button>
                <button className="bg-[#0099cc] hover:bg-[#0088bb] text-white p-1 rounded-sm w-6 h-6 flex items-center justify-center">
                  <ChevronRight size={14} />
                </button>
                <button className="bg-[#4caf50] hover:bg-[#45a049] text-white px-3 py-1 rounded-sm text-xs font-bold uppercase transition">
                  Add
                </button>
                <button className="bg-[#f44336] hover:bg-[#d32f2f] text-white px-3 py-1 rounded-sm text-xs font-bold uppercase transition">
                  delete
                </button>
                <button className="bg-[#4caf50] hover:bg-[#45a049] text-white px-3 py-1 rounded-sm text-xs font-bold uppercase transition ml-2">
                  Save
                </button>

              </div>
            </div>

            {/* Timeline Slider Mockup */}
            <div className="relative w-full h-8 bg-gray-100 rounded-full border border-blue-200 mt-4 flex items-center px-1">
              <div className="w-full h-1 bg-blue-500 rounded-full relative">
                <div className="absolute right-0 top-1/2 transform -translate-y-1/2 w-3 h-3 bg-blue-600 rounded-full border-2 border-white shadow cursor-pointer"></div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
