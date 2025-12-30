# Data Collection & Vault Building Guide

## Overview
This guide explains how to build and maintain your offline knowledge vault for the Survival AI app.

## What You Need to Provide

### 1. Wikipedia ZIM File

**What**: Compressed Wikipedia dump for offline browsing
**Where to get**: https://wiki.kiwix.org/wiki/Content_in_all_languages
**Recommended files**:
- `wikipedia_en_all_maxi_2024-01.zim` (Full Wikipedia, ~95GB)
- `wikipedia_en_all_mini_2024-01.zim` (Smaller subset, ~40GB)
- `wikipedia_en_simple_all_2024-01.zim` (Simple English, ~1GB)

**How to install**:
1. Download ZIM file
2. Copy to `ux0:data/survivalkit/zim/`
3. Rename to `wikipedia_en.zim` OR update code to match filename

**Performance tips**:
- Larger ZIMs take longer to search but have more content
- Mini version is recommended for best performance on Vita
- Simple English version is fastest but has less detailed content

### 2. Survival & Reference Content

**Curated Guides** (text/HTML/PDF):
- First aid manuals
- Wilderness survival guides
- Emergency preparedness documents
- Military field manuals (public domain)
- Bushcraft guides
- Navigation techniques
- Water purification methods
- Shelter building instructions
- Fire starting techniques
- Foraging guides (with safety warnings)

**Recommended sources** (public domain/CC):
- US Army Survival Manual (FM 21-76)
- SAS Survival Handbook excerpts (check license)
- Wilderness Medicine guides
- Red Cross First Aid guides
- FEMA emergency preparedness guides

**How to add**:
1. Convert to plain text
2. Place in `ux0:data/survivalkit/manuals/`
3. App will index and make searchable

### 3. Vault Data (Web References)

Use the PC Collector tool to build your vault database.

**Example topics to collect**:
- Recent survival techniques
- First aid updates
- Emergency preparedness news
- Bushcraft tutorials
- Medical reference articles
- Navigation guides
- Tool and equipment reviews

**Safe/Legal sources**:
- Government websites (CDC, FEMA, etc.)
- Wikipedia articles
- Creative Commons blogs
- Open educational resources
- Public domain materials
- RSS feeds from allowed sources

**NOT recommended** (copyright/legal issues):
- Full copyrighted articles from news sites
- Paywalled content
- Proprietary medical texts
- Commercial survival guides

### 4. Voice Pack (Optional)

See `VOICE_PACK_GUIDE.md` for details on creating a voice pack.

**Quick start**:
```bash
cd tools
python generate_voice_pack.py --method espeak --output voice_pack/
```

Copy to: `ux0:data/survivalkit/voice/pack/`

## Using PC Collector

### Basic Usage

**Collect from URLs**:
```bash
python pc_collector.py --output vault_pack --urls \
  "https://www.cdc.gov/firstaid/basics/index.html" \
  "https://www.rei.com/learn/expert-advice/survival-skills.html" \
  --tags survival first-aid
```

**Collect from RSS feed**:
```bash
python pc_collector.py --output vault_pack \
  --rss "https://example.com/survival/feed.xml" \
  --tags survival --limit 20
```

**Check statistics**:
```bash
python pc_collector.py --output vault_pack --stats
```

### Building a Complete Vault

**Step 1: Initial collection**
```bash
# Survival topics
python pc_collector.py --output vault_pack --urls \
  "https://www.survival-expert.com/guide" \
  --tags survival wilderness

# First aid
python pc_collector.py --output vault_pack --urls \
  "https://www.redcross.org/first-aid" \
  --tags first-aid medical

# Navigation
python pc_collector.py --output vault_pack --urls \
  "https://www.outdoor-guide.com/navigation" \
  --tags navigation orienteering
```

**Step 2: Regular updates**
Create a collection script `update_vault.sh`:
```bash
#!/bin/bash
# Update vault with latest content

python pc_collector.py --output vault_pack \
  --rss "https://survival-blog.com/feed" \
  --tags survival --limit 10

python pc_collector.py --output vault_pack \
  --rss "https://first-aid-news.com/feed" \
  --tags first-aid medical --limit 10

# Optimize after updates
python pc_collector.py --output vault_pack --stats
```

Run weekly: `./update_vault.sh`

**Step 3: Transfer to Vita**
```bash
# Copy database
cp vault_pack/vault.sqlite /path/to/vita/ux0/data/survivalkit/db/

# Copy items
cp -r vault_pack/items/* /path/to/vita/ux0/data/survivalkit/vault/items/
```

### Vault Maintenance

**Pruning old items**:
```python
# Remove items older than 1 year
import sqlite3
import time

conn = sqlite3.connect('vault.sqlite')
one_year_ago = int(time.time()) - (365 * 24 * 60 * 60)

conn.execute('DELETE FROM items WHERE retrieved_at < ?', (one_year_ago,))
conn.commit()
```

**Deduplication**:
```python
# Remove duplicate URLs
conn.execute('''
DELETE FROM items WHERE rowid NOT IN (
  SELECT MIN(rowid) FROM items GROUP BY url
)
''')
conn.commit()
```

## Content Curation Tips

### Quality over Quantity
- Better to have 100 high-quality sources than 1000 poor ones
- Prioritize authoritative sources (government, medical, educational)
- Verify information from multiple sources

### Topic Coverage
Essential topics to cover:
- First aid (bleeding, burns, fractures, CPR)
- Water (purification, finding sources)
- Shelter (building, insulation, location)
- Fire (starting methods, tinder, safety)
- Food (foraging safety, preservation)
- Navigation (compass, stars, landmarks)
- Signaling (SOS, mirrors, smoke)
- Weather (reading signs, safety)
- Wildlife (dangerous animals, avoidance)
- Emergency preparedness (kits, planning)

### Tagging Strategy
Use consistent tags for better search:
- **Skill level**: beginner, intermediate, advanced
- **Environment**: desert, forest, arctic, ocean, urban
- **Season**: summer, winter, all-season
- **Category**: water, shelter, fire, food, medical, navigation
- **Urgency**: emergency, preventive, routine

Example: `--tags survival,water,emergency,desert,beginner`

### Copyright Compliance

**Always allowed**:
- Government publications (usually public domain)
- Creative Commons licensed content
- Wikipedia articles
- Public domain materials

**With attribution**:
- CC-BY licensed content (give credit)
- Educational fair use excerpts (with citation)

**NOT allowed**:
- Full copyrighted articles without permission
- Paywalled content
- Commercial ebooks/guides
- Proprietary medical texts

**Best practice**: Store metadata + snippet + URL instead of full text when uncertain.

## Example Vault Structure

After following this guide, you should have:

```
ux0:data/survivalkit/
├── zim/
│   └── wikipedia_en.zim (40GB)
├── vault/
│   ├── items/ (500+ reference files)
│   └── media/ (optional images)
├── db/
│   └── vault.sqlite (10MB with 500+ indexed items)
├── manuals/
│   ├── army_survival_manual.txt
│   ├── first_aid_guide.txt
│   └── wilderness_medicine.txt
└── voice/
    └── pack/ (50MB voice pack, optional)
```

**Total size**: ~45-100GB depending on Wikipedia choice and vault size

## Questions?

- **Q: Do I need all this data?**
  A: No! Start with just Wikipedia ZIM. Add vault and voice gradually.

- **Q: Can I use mobile data to update vault?**
  A: Not recommended. PC Collector is meant for WiFi/desktop use.

- **Q: How often should I update?**
  A: Monthly for general content, weekly for time-sensitive topics.

- **Q: Can I share my vault pack?**
  A: Yes, if all content is properly licensed and attributed.

## Next Steps

1. Download Wikipedia ZIM file
2. Copy to Vita
3. Test basic Wikipedia search
4. Use PC Collector to add survival content
5. (Optional) Create voice pack
6. Build your knowledge vault over time!
