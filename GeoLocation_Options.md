# Geolocation-Based Business Search: Open Source Options

This document outlines open source solutions for generating comprehensive lists of local businesses given a specified geographic location (longitude/latitude).

---

## 1. Data Sources

### 1.1 OpenStreetMap (OSM)

**Overview:** Community-maintained global geographic database with extensive POI (Points of Interest) data including businesses.

**Pros:**
- Completely free and open (ODbL license)
- Global coverage
- Rich tagging system for business categories
- Active community maintaining data

**Cons:**
- Variable data quality/completeness by region
- No ratings, reviews, or operational metrics
- May lack newer businesses

**Access Methods:**

| Method | Description | Best For |
|--------|-------------|----------|
| **Overpass API** | Query OSM data with OverpassQL | Real-time queries, small areas |
| **Nominatim** | Geocoding/reverse geocoding | Address lookup |
| **OSM Data Dumps** | Full planet file or regional extracts | Bulk processing, offline use |

**Example Overpass Query (businesses within radius):**
```
[out:json];
(
  node["shop"](around:5000, 39.7817, -89.6501);
  node["amenity"~"restaurant|cafe|bank|pharmacy"](around:5000, 39.7817, -89.6501);
  node["office"](around:5000, 39.7817, -89.6501);
);
out body;
```

**Resources:**
- Overpass API: https://overpass-api.de/
- Overpass Turbo (testing): https://overpass-turbo.eu/
- Nominatim: https://nominatim.org/
- Regional Extracts: https://download.geofabrik.de/

---

### 1.2 Wikidata

**Overview:** Structured knowledge base with business entities, linked to Wikipedia articles.

**Pros:**
- Rich semantic data (founding dates, industry codes, parent companies)
- Links to other datasets (OSM, official websites)
- SPARQL query interface

**Cons:**
- Limited coverage of small/local businesses
- Better for larger chains and notable businesses

**Example SPARQL Query:**
```sparql
SELECT ?business ?businessLabel ?coord WHERE {
  ?business wdt:P31/wdt:P279* wd:Q4830453.  # instance of business
  ?business wdt:P625 ?coord.                  # has coordinates
  SERVICE wikibase:around {
    ?business wdt:P625 ?coord.
    bd:serviceParam wikibase:center "Point(-89.6501 39.7817)"^^geo:wktLiteral.
    bd:serviceParam wikibase:radius "10".     # km
  }
  SERVICE wikibase:label { bd:serviceParam wikibase:language "en". }
}
```

**Resources:**
- Query Service: https://query.wikidata.org/
- Documentation: https://www.wikidata.org/wiki/Wikidata:Data_access

---

### 1.3 OpenCage Geocoding API

**Overview:** Aggregates multiple open data sources (OSM, government datasets) for geocoding.

**Pros:**
- Free tier (2,500 requests/day)
- Combines multiple open datasets
- Good address normalization

**Cons:**
- Not specifically for business search
- Rate limits on free tier

**Resources:**
- API: https://opencagedata.com/
- GitHub: https://github.com/OpenCageData

---

## 2. Self-Hosted Search Infrastructure

### 2.1 Pelias Geocoder

**Overview:** Modular, open source geocoder built on Elasticsearch.

**Pros:**
- Full control over data and infrastructure
- Combines OSM, Who's on First, OpenAddresses, Geonames
- Fast autocomplete and search
- Docker deployment available

**Cons:**
- Requires infrastructure to host
- Setup complexity

**Data Sources Supported:**
- OpenStreetMap
- OpenAddresses
- Who's on First (admin boundaries)
- Geonames

**Resources:**
- GitHub: https://github.com/pelias/pelias
- Docker Setup: https://github.com/pelias/docker
- Documentation: https://github.com/pelias/documentation

---

### 2.2 Photon (by Komoot)

**Overview:** Open source geocoder built for OpenStreetMap data, powered by Elasticsearch.

**Pros:**
- Lightweight and fast
- Built specifically for OSM
- Supports multiple languages
- Easy Docker deployment

**Cons:**
- OSM data only
- Less feature-rich than Pelias

**Resources:**
- GitHub: https://github.com/komoot/photon
- Demo: https://photon.komoot.io/

---

### 2.3 Typesense / Meilisearch

**Overview:** Open source search engines that can be used to build custom geospatial business search.

**Use Case:** Import business data from OSM/other sources and create a fast, typo-tolerant search with geo-filtering.

**Typesense Features:**
- Built-in geo-search (filter by radius, sort by distance)
- Typo tolerance
- Faceting by category
- Fast (<50ms searches)

**Resources:**
- Typesense: https://github.com/typesense/typesense
- Meilisearch: https://github.com/meilisearch/meilisearch

---

## 3. AI/ML Enhancement (Open Source)

### 3.1 Sentence Transformers (for Semantic Search)

**Overview:** Convert business descriptions/categories to embeddings for semantic matching.

**Use Case:** User searches "places to grab lunch" → matches restaurants, cafes, delis, food trucks.

**Models:**
- `all-MiniLM-L6-v2` - Fast, good quality
- `all-mpnet-base-v2` - Higher quality, slower

**Resources:**
- GitHub: https://github.com/UKPLab/sentence-transformers
- Hugging Face: https://huggingface.co/sentence-transformers

---

### 3.2 SpaCy + NER for Business Extraction

**Overview:** Extract business names and addresses from unstructured text (web scraping, documents).

**Resources:**
- SpaCy: https://github.com/explosion/spaCy
- Custom NER training: https://spacy.io/usage/training

---

### 3.3 Dedupe (Entity Resolution)

**Overview:** ML library for deduplicating and linking records from multiple sources.

**Use Case:** Merge business records from OSM, scraped data, and other sources.

**Resources:**
- GitHub: https://github.com/dedupeio/dedupe
- Documentation: https://docs.dedupe.io/

---

## 4. Web Scraping (Supplementary Data)

### 4.1 Scrapy

**Overview:** Python web scraping framework for collecting business data from public websites.

**Ethical Considerations:**
- Respect robots.txt
- Rate limit requests
- Check terms of service

**Resources:**
- GitHub: https://github.com/scrapy/scrapy

---

### 4.2 Common Crawl

**Overview:** Open repository of web crawl data (petabytes of archived web pages).

**Use Case:** Extract business information from cached web pages at scale.

**Resources:**
- https://commoncrawl.org/

---

## 5. Recommended Architecture (Open Source Stack)

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Query                                │
│                  (lat/lon + search terms)                        │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Pelias or Photon                            │
│                    (Geocoding Layer)                             │
│              - Address normalization                             │
│              - Coordinate lookup                                 │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                 Typesense / Meilisearch                          │
│                   (Search Engine)                                │
│         - Geo-filtered business search                           │
│         - Faceting by category                                   │
│         - Typo-tolerant matching                                 │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│              Sentence Transformers (Optional)                    │
│                  (Semantic Ranking)                              │
│         - Re-rank results by semantic relevance                  │
│         - Natural language query understanding                   │
└─────────────────────────────┬───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Ranked Results                               │
│            (Businesses sorted by relevance)                      │
└─────────────────────────────────────────────────────────────────┘
```

---

## 6. Data Pipeline for OSM Business Import

```bash
# 1. Download regional OSM extract
wget https://download.geofabrik.de/north-america/us/illinois-latest.osm.pbf

# 2. Filter for POIs using osmium
osmium tags-filter illinois-latest.osm.pbf \
  nwr/shop nwr/amenity nwr/office nwr/tourism nwr/leisure \
  -o illinois-pois.osm.pbf

# 3. Convert to GeoJSON
osmium export illinois-pois.osm.pbf -o illinois-pois.geojson

# 4. Import into search engine (Typesense example)
# Use custom script to parse GeoJSON and index documents
```

---

## 7. Comparison Matrix

| Solution | Cost | Coverage | Data Quality | Setup Effort | Real-time |
|----------|------|----------|--------------|--------------|-----------|
| Overpass API | Free | Global | Variable | Low | Yes |
| Pelias (self-hosted) | Infra only | Global | Good | High | Yes |
| Photon | Free/Infra | Global | Good | Medium | Yes |
| Typesense + OSM | Infra only | Global | Variable | Medium | Yes |
| Wikidata | Free | Limited | High | Low | Yes |

---

## 8. Next Steps

1. **Evaluate data coverage** - Query Overpass API for your target regions to assess OSM data completeness
2. **Prototype with Overpass** - Build initial search using Overpass API for validation
3. **Deploy Typesense** - Set up search infrastructure for production-grade performance
4. **Implement semantic search** - Add sentence transformers for natural language queries
5. **Build data pipeline** - Automate OSM data updates (weekly/monthly refresh)

---

## 9. Useful Links

- OSM Wiki (Map Features): https://wiki.openstreetmap.org/wiki/Map_features
- OSM Taginfo (tag statistics): https://taginfo.openstreetmap.org/
- Overpass Query Examples: https://wiki.openstreetmap.org/wiki/Overpass_API/Overpass_API_by_Example
- GeoJSON Specification: https://geojson.org/
