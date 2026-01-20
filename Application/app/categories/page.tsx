'use client';

import Link from 'next/link';
import { useState } from 'react';
import { Settings, Plus, Edit2, Trash2, Tag, ChevronRight, Zap } from 'lucide-react';


export default function CategoriesPage() {
    const [categories, setCategories] = useState([
        { id: 1, name: 'Go-Kart', description: 'Racing karts and rental karts', count: 12 },
        { id: 2, name: 'Motorcycle', description: 'Track bikes and street bikes', count: 5 },
        { id: 3, name: 'Car', description: 'Track day cars and race cars', count: 3 },
    ]);

    return (
        <div className="min-h-screen bg-background text-foreground pb-24">


            {/* Content */}
            <div className="container mx-auto px-4 py-6">

                <div className="flex justify-end mb-6">
                    <button className="bg-primary hover:bg-primary-hover text-white px-4 py-2 rounded-lg transition font-racing text-sm flex items-center gap-2">
                        <Plus className="w-4 h-4" />
                        ADD CATEGORY
                    </button>
                </div>

                <div className="grid gap-4">
                    {categories.map((category) => (
                        <div key={category.id} className="carbon-bg border border-border-color rounded-xl p-4 flex items-center justify-between group hover:border-primary/50 transition">
                            <div className="flex items-center gap-4">
                                <div className="w-12 h-12 bg-background-secondary rounded-lg flex items-center justify-center">
                                    <Zap className="w-6 h-6 text-highlight" />
                                </div>
                                <div>
                                    <h3 className="text-foreground font-racing text-lg">{category.name}</h3>
                                    <p className="text-text-secondary text-xs">{category.description}</p>
                                </div>
                            </div>

                            <div className="flex items-center gap-4">
                                <div className="text-center md:mr-4">
                                    <div className="text-foreground font-data font-bold text-lg">{category.count}</div>
                                    <div className="text-text-secondary text-[10px] uppercase">Sessions</div>
                                </div>

                                <div className="flex gap-2 opacity-0 group-hover:opacity-100 transition-opacity">
                                    <button className="p-2 hover:bg-background-secondary rounded text-text-secondary hover:text-white transition">
                                        <Edit2 className="w-4 h-4" />
                                    </button>
                                    <button className="p-2 hover:bg-background-secondary rounded text-text-secondary hover:text-warning transition">
                                        <Trash2 className="w-4 h-4" />
                                    </button>
                                </div>
                            </div>
                        </div>
                    ))}
                </div>

            </div>

        </div>
    );
}
