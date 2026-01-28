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

## 6. Agentic Architecture

An alternative to the traditional pipeline architecture is an **agentic approach** where each layer operates as an autonomous service orchestrated by an LLM. This enables dynamic routing, fault tolerance, and natural language interfaces.

### 6.1 Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         User Natural Language Query                      │
│            "Find me restaurants near downtown Springfield IL             │
│                    that might need catering services"                    │
└─────────────────────────────────┬───────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                          LLM ORCHESTRATOR                                │
│                                                                          │
│   • Parses user intent                                                   │
│   • Selects appropriate agents                                           │
│   • Determines execution order (parallel vs sequential)                  │
│   • Synthesizes results from multiple agents                             │
│   • Handles errors and retries                                           │
└───────┬─────────────────┬─────────────────┬─────────────────┬───────────┘
        │                 │                 │                 │
        ▼                 ▼                 ▼                 ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│  Geocoding   │  │   OSM Data   │  │  Wikidata    │  │  Web Search  │
│    Agent     │  │    Agent     │  │    Agent     │  │    Agent     │
│              │  │              │  │              │  │              │
│ • Nominatim  │  │ • Overpass   │  │ • SPARQL     │  │ • SearXNG    │
│ • Pelias     │  │ • POI query  │  │ • Enrichment │  │ • Scraping   │
│ • Photon     │  │ • Categories │  │ • Firmograph │  │ • Validation │
└──────┬───────┘  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘
       │                 │                 │                 │
       └─────────────────┴────────┬────────┴─────────────────┘
                                  │
                                  ▼
                    ┌──────────────────────────┐
                    │   Entity Resolution      │
                    │        Agent             │
                    │                          │
                    │  • Deduplicate records   │
                    │  • Merge attributes      │
                    │  • Confidence scoring    │
                    └────────────┬─────────────┘
                                 │
                                 ▼
                    ┌──────────────────────────┐
                    │   Semantic Ranking       │
                    │        Agent             │
                    │                          │
                    │  • Embedding similarity  │
                    │  • Intent matching       │
                    │  • Lead scoring          │
                    └────────────┬─────────────┘
                                 │
                                 ▼
                    ┌──────────────────────────┐
                    │     Ranked Results       │
                    │   (Structured Output)    │
                    └──────────────────────────┘
```

### 6.2 Agent Definitions

Each agent is a self-contained service with defined inputs, outputs, and tools.

#### Geocoding Agent

| Property | Description |
|----------|-------------|
| **Purpose** | Convert addresses/place names to coordinates and vice versa |
| **Input** | `{ query: string, type: "forward" \| "reverse", lat?: number, lon?: number }` |
| **Output** | `{ lat: number, lon: number, display_name: string, confidence: number }` |
| **Tools** | Nominatim API, Pelias API, Photon API |
| **Fallback** | Try alternative geocoder if primary fails |

#### OSM Data Agent

| Property | Description |
|----------|-------------|
| **Purpose** | Query OpenStreetMap for POIs within a geographic area |
| **Input** | `{ lat: number, lon: number, radius_m: number, categories?: string[] }` |
| **Output** | `{ businesses: [{ name, lat, lon, category, address, phone, website, osm_id }] }` |
| **Tools** | Overpass API, osmium CLI |
| **Capabilities** | Category filtering, radius search, tag extraction |

#### Wikidata Agent

| Property | Description |
|----------|-------------|
| **Purpose** | Enrich business records with structured knowledge |
| **Input** | `{ business_name: string, lat: number, lon: number }` |
| **Output** | `{ wikidata_id, industry, employee_count, founding_date, parent_company, website }` |
| **Tools** | SPARQL endpoint, entity reconciliation API |
| **Use Case** | Firmographic data for larger/notable businesses |

#### Web Search Agent

| Property | Description |
|----------|-------------|
| **Purpose** | Find supplementary business information from the web |
| **Input** | `{ business_name: string, location: string, info_needed: string[] }` |
| **Output** | `{ website, social_media, recent_news, contact_info }` |
| **Tools** | SearXNG (self-hosted), Scrapy, Common Crawl |
| **Constraints** | Rate limiting, robots.txt compliance |

#### Entity Resolution Agent

| Property | Description |
|----------|-------------|
| **Purpose** | Merge duplicate records from multiple sources |
| **Input** | `{ records: BusinessRecord[] }` |
| **Output** | `{ merged_records: BusinessRecord[], duplicates_found: number }` |
| **Tools** | Dedupe library, fuzzy matching (RapidFuzz) |
| **Algorithm** | ML-based record linkage with confidence scores |

#### Semantic Ranking Agent

| Property | Description |
|----------|-------------|
| **Purpose** | Rank results by relevance to user intent |
| **Input** | `{ query: string, businesses: BusinessRecord[] }` |
| **Output** | `{ ranked_businesses: BusinessRecord[], relevance_scores: number[] }` |
| **Tools** | Sentence Transformers, CrossEncoder models |
| **Model** | `all-MiniLM-L6-v2` or `ms-marco-MiniLM-L-6-v2` |

### 6.3 LLM Orchestrator

The orchestrator is the central intelligence that coordinates agents using function calling.

**Responsibilities:**
- Parse natural language queries into structured intent
- Determine which agents to invoke (and in what order)
- Execute agents in parallel when possible
- Handle agent failures with retries or fallbacks
- Synthesize final results with explanations

**Example Orchestration Flow:**

```python
# Pseudo-code for LLM orchestrator logic

def orchestrate(user_query: str) -> SearchResults:
    # Step 1: LLM parses intent
    intent = llm.parse_intent(user_query)
    # → { location: "downtown Springfield IL",
    #     business_type: "restaurants",
    #     purpose: "catering prospects" }

    # Step 2: Geocode location (required first)
    geo_result = geocoding_agent.execute(intent.location)

    # Step 3: Query data sources in PARALLEL
    osm_future = osm_agent.execute_async(geo_result, intent.business_type)
    wiki_future = wikidata_agent.execute_async(geo_result, intent.business_type)
    web_future = web_agent.execute_async(intent)  # optional enrichment

    # Step 4: Gather results
    all_records = await gather(osm_future, wiki_future, web_future)

    # Step 5: Deduplicate and merge
    merged = entity_resolution_agent.execute(all_records)

    # Step 6: Rank by relevance to original query
    ranked = semantic_ranking_agent.execute(user_query, merged)

    # Step 7: LLM synthesizes final response
    return llm.synthesize_response(ranked, user_query)
```

### 6.4 Agent Tool Schemas (OpenAI Function Calling Format)

These schemas enable LLMs to invoke agents as tools:

```json
{
  "name": "geocoding_agent",
  "description": "Convert a place name or address to geographic coordinates",
  "parameters": {
    "type": "object",
    "properties": {
      "query": {
        "type": "string",
        "description": "Address or place name to geocode"
      },
      "type": {
        "type": "string",
        "enum": ["forward", "reverse"],
        "description": "Forward (text→coords) or reverse (coords→text)"
      }
    },
    "required": ["query", "type"]
  }
}
```

```json
{
  "name": "osm_data_agent",
  "description": "Search OpenStreetMap for businesses near a location",
  "parameters": {
    "type": "object",
    "properties": {
      "lat": { "type": "number", "description": "Latitude" },
      "lon": { "type": "number", "description": "Longitude" },
      "radius_m": { "type": "integer", "description": "Search radius in meters" },
      "categories": {
        "type": "array",
        "items": { "type": "string" },
        "description": "OSM category tags (e.g., 'restaurant', 'office', 'shop')"
      }
    },
    "required": ["lat", "lon", "radius_m"]
  }
}
```

```json
{
  "name": "semantic_ranking_agent",
  "description": "Rank businesses by relevance to a natural language query",
  "parameters": {
    "type": "object",
    "properties": {
      "query": {
        "type": "string",
        "description": "Natural language description of ideal business"
      },
      "business_ids": {
        "type": "array",
        "items": { "type": "string" },
        "description": "IDs of businesses to rank"
      }
    },
    "required": ["query", "business_ids"]
  }
}
```

### 6.5 Open Source Frameworks for Agentic Systems

| Framework | Language | Description | Best For |
|-----------|----------|-------------|----------|
| **LangChain** | Python/JS | Most popular agent framework | Rapid prototyping |
| **LangGraph** | Python | Stateful multi-agent workflows | Complex orchestration |
| **CrewAI** | Python | Role-based multi-agent collaboration | Team-like agent interactions |
| **AutoGen** | Python | Microsoft's multi-agent framework | Conversational agents |
| **Semantic Kernel** | C#/Python | Microsoft's LLM orchestration SDK | Enterprise/.NET integration |
| **Haystack** | Python | NLP pipeline framework | Search/RAG applications |
| **DSPy** | Python | Programmatic LLM pipelines | Optimized prompt engineering |

**Recommended Stack:**
- **LangGraph** for orchestration (supports cycles, parallel execution, state)
- **FastAPI** for agent HTTP endpoints
- **Redis** for inter-agent message passing
- **PostgreSQL + pgvector** for embeddings storage

### 6.6 Example: LangGraph Implementation

```python
from langgraph.graph import StateGraph, END
from typing import TypedDict, List

class SearchState(TypedDict):
    query: str
    coordinates: dict | None
    osm_results: List[dict]
    wiki_results: List[dict]
    merged_results: List[dict]
    ranked_results: List[dict]

# Define the graph
workflow = StateGraph(SearchState)

# Add agent nodes
workflow.add_node("geocode", geocoding_agent)
workflow.add_node("osm_search", osm_agent)
workflow.add_node("wiki_search", wikidata_agent)
workflow.add_node("merge", entity_resolution_agent)
workflow.add_node("rank", semantic_ranking_agent)

# Define edges (execution flow)
workflow.set_entry_point("geocode")
workflow.add_edge("geocode", "osm_search")
workflow.add_edge("geocode", "wiki_search")  # Parallel execution
workflow.add_edge("osm_search", "merge")
workflow.add_edge("wiki_search", "merge")
workflow.add_edge("merge", "rank")
workflow.add_edge("rank", END)

# Compile and run
app = workflow.compile()
result = app.invoke({"query": "restaurants in Springfield IL"})
```

### 6.7 Benefits of Agentic Architecture

| Benefit | Description |
|---------|-------------|
| **Dynamic Routing** | LLM decides which agents to call based on query type |
| **Graceful Degradation** | If one data source fails, others can still provide results |
| **Extensibility** | Add new agents (e.g., Yelp agent) without changing orchestration |
| **Explainability** | LLM can explain why certain results were ranked higher |
| **Natural Language** | Users don't need to learn query syntax |
| **Parallel Execution** | Independent agents run concurrently for faster results |
| **Self-Healing** | LLM can retry failed agents or try alternative approaches |

### 6.8 Considerations

| Challenge | Mitigation |
|-----------|------------|
| **Latency** | Cache agent results, use async execution |
| **Cost** | Use smaller models for orchestration, cache LLM responses |
| **Reliability** | Implement circuit breakers, timeouts, fallbacks |
| **Observability** | Log all agent calls, trace execution paths |
| **Testing** | Mock agents for unit tests, integration tests with real data |

---

## 7. Data Pipeline for OSM Business Import

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

## 8. Comparison Matrix

| Solution | Cost | Coverage | Data Quality | Setup Effort | Real-time |
|----------|------|----------|--------------|--------------|-----------|
| Overpass API | Free | Global | Variable | Low | Yes |
| Pelias (self-hosted) | Infra only | Global | Good | High | Yes |
| Photon | Free/Infra | Global | Good | Medium | Yes |
| Typesense + OSM | Infra only | Global | Variable | Medium | Yes |
| Wikidata | Free | Limited | High | Low | Yes |

---

## 9. Next Steps

1. **Evaluate data coverage** - Query Overpass API for your target regions to assess OSM data completeness
2. **Prototype with Overpass** - Build initial search using Overpass API for validation
3. **Deploy Typesense** - Set up search infrastructure for production-grade performance
4. **Implement semantic search** - Add sentence transformers for natural language queries
5. **Build data pipeline** - Automate OSM data updates (weekly/monthly refresh)

---

## 10. Useful Links

- OSM Wiki (Map Features): https://wiki.openstreetmap.org/wiki/Map_features
- OSM Taginfo (tag statistics): https://taginfo.openstreetmap.org/
- Overpass Query Examples: https://wiki.openstreetmap.org/wiki/Overpass_API/Overpass_API_by_Example
- GeoJSON Specification: https://geojson.org/
