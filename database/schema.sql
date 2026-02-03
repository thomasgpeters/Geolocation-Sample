-- ============================================================================
-- FranchiseAI Prospect Discovery Platform - PostgreSQL Schema
-- ApiLogicServer Data Model for Prospect Tracking
-- ============================================================================

-- Enable required extensions (no PostGIS needed)
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ============================================================================
-- ENUM TYPES
-- ============================================================================

CREATE TYPE prospect_status AS ENUM (
    'new',
    'contacted',
    'qualified',
    'proposal_sent',
    'negotiating',
    'won',
    'lost',
    'inactive'
);

CREATE TYPE business_size AS ENUM (
    'micro',        -- 1-9 employees
    'small',        -- 10-49 employees
    'medium',       -- 50-249 employees
    'large',        -- 250+ employees
    'enterprise'    -- 1000+ employees
);

CREATE TYPE outreach_type AS ENUM (
    'email',
    'phone',
    'in_person',
    'linkedin',
    'mail',
    'referral',
    'event'
);

CREATE TYPE score_source AS ENUM (
    'ai_model',
    'manual',
    'rule_based',
    'imported'
);

-- ============================================================================
-- CORE TABLES
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Regions: Top-level geographic areas
-- ---------------------------------------------------------------------------
CREATE TABLE regions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name VARCHAR(100) NOT NULL,
    code VARCHAR(20) UNIQUE NOT NULL,
    country_code CHAR(2) NOT NULL DEFAULT 'US',
    timezone VARCHAR(50) DEFAULT 'America/New_York',

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

COMMENT ON TABLE regions IS 'Top-level geographic regions for franchise organization';

-- ---------------------------------------------------------------------------
-- Territories: Assignable sales territories within regions
-- ---------------------------------------------------------------------------
CREATE TABLE territories (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    region_id UUID NOT NULL REFERENCES regions(id) ON DELETE CASCADE,

    name VARCHAR(100) NOT NULL,
    code VARCHAR(30) UNIQUE NOT NULL,
    description TEXT,

    -- Center point (simple lat/lng)
    center_latitude DECIMAL(10, 8),
    center_longitude DECIMAL(11, 8),

    -- Territory metadata
    radius_miles DECIMAL(10, 2),
    zip_codes TEXT[],  -- Array of ZIP codes in territory

    -- Status
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_territories_region ON territories(region_id);

COMMENT ON TABLE territories IS 'Sales territories with geographic boundaries';

-- ---------------------------------------------------------------------------
-- Territory Demographics: Demographic data for each territory
-- ---------------------------------------------------------------------------
CREATE TABLE territory_demographics (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    territory_id UUID NOT NULL REFERENCES territories(id) ON DELETE CASCADE,

    -- Population data
    total_population INTEGER,
    population_density DECIMAL(10, 2),  -- per square mile
    median_age DECIMAL(4, 1),

    -- Household data
    total_households INTEGER,
    median_household_income DECIMAL(12, 2),
    average_household_size DECIMAL(3, 1),

    -- Income distribution
    income_under_25k_pct DECIMAL(5, 2),
    income_25k_50k_pct DECIMAL(5, 2),
    income_50k_75k_pct DECIMAL(5, 2),
    income_75k_100k_pct DECIMAL(5, 2),
    income_100k_150k_pct DECIMAL(5, 2),
    income_over_150k_pct DECIMAL(5, 2),

    -- Business data
    total_businesses INTEGER,
    total_employees INTEGER,
    business_density DECIMAL(10, 2),  -- businesses per square mile

    -- Market indicators
    daytime_population INTEGER,
    commuter_inflow INTEGER,
    commuter_outflow INTEGER,

    -- Data freshness
    data_year INTEGER NOT NULL,
    data_source VARCHAR(100),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

    UNIQUE(territory_id, data_year)
);

CREATE INDEX idx_territory_demographics_territory ON territory_demographics(territory_id);

COMMENT ON TABLE territory_demographics IS 'Demographic and market data per territory';

-- ---------------------------------------------------------------------------
-- Franchisees: Franchise owners/operators
-- ---------------------------------------------------------------------------
CREATE TABLE franchisees (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    -- Business info
    business_name VARCHAR(200) NOT NULL,
    dba_name VARCHAR(200),
    franchise_number VARCHAR(50) UNIQUE,

    -- Contact info
    owner_first_name VARCHAR(100),
    owner_last_name VARCHAR(100),
    email VARCHAR(255),
    phone VARCHAR(30),

    -- Address
    address_line1 VARCHAR(200),
    address_line2 VARCHAR(100),
    city VARCHAR(100),
    state_province VARCHAR(50),
    postal_code VARCHAR(20),
    country_code CHAR(2) DEFAULT 'US',

    -- Geolocation (simple lat/lng)
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),

    -- Franchise details
    start_date DATE,
    contract_end_date DATE,
    is_active BOOLEAN DEFAULT true,

    -- Performance metrics
    monthly_revenue_target DECIMAL(12, 2),
    ytd_revenue DECIMAL(14, 2),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_franchisees_active ON franchisees(is_active) WHERE is_active = true;

COMMENT ON TABLE franchisees IS 'Franchise owners and their business details';

-- ---------------------------------------------------------------------------
-- Franchisee Territories: Many-to-many relationship
-- ---------------------------------------------------------------------------
CREATE TABLE franchisee_territories (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    franchisee_id UUID NOT NULL REFERENCES franchisees(id) ON DELETE CASCADE,
    territory_id UUID NOT NULL REFERENCES territories(id) ON DELETE CASCADE,

    is_primary BOOLEAN DEFAULT false,
    assigned_date DATE DEFAULT CURRENT_DATE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

    UNIQUE(franchisee_id, territory_id)
);

CREATE INDEX idx_franchisee_territories_franchisee ON franchisee_territories(franchisee_id);
CREATE INDEX idx_franchisee_territories_territory ON franchisee_territories(territory_id);

COMMENT ON TABLE franchisee_territories IS 'Assignment of territories to franchisees';

-- ---------------------------------------------------------------------------
-- Industries: Industry classification for prospects
-- ---------------------------------------------------------------------------
CREATE TABLE industries (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    name VARCHAR(100) NOT NULL,
    naics_code VARCHAR(10),
    sic_code VARCHAR(10),

    -- Catering relevance
    catering_potential_score INTEGER CHECK (catering_potential_score BETWEEN 1 AND 10),
    typical_order_size VARCHAR(50),
    peak_seasons TEXT[],

    parent_id UUID REFERENCES industries(id),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_industries_naics ON industries(naics_code);
CREATE INDEX idx_industries_parent ON industries(parent_id);

COMMENT ON TABLE industries IS 'Industry classification with catering relevance scoring';

-- ---------------------------------------------------------------------------
-- Prospects: Prospective catering clients
-- ---------------------------------------------------------------------------
CREATE TABLE prospects (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    -- Assignment
    territory_id UUID REFERENCES territories(id),
    franchisee_id UUID REFERENCES franchisees(id),
    assigned_to_user_id UUID,  -- Sales rep user ID

    -- Business identification
    business_name VARCHAR(300) NOT NULL,
    dba_name VARCHAR(300),
    legal_name VARCHAR(300),

    -- Industry
    industry_id UUID REFERENCES industries(id),
    industry_naics VARCHAR(10),
    business_type VARCHAR(100),

    -- Business size
    employee_count INTEGER,
    employee_count_range business_size,
    annual_revenue DECIMAL(14, 2),
    year_established INTEGER,

    -- Address
    address_line1 VARCHAR(200),
    address_line2 VARCHAR(100),
    city VARCHAR(100),
    state_province VARCHAR(50),
    postal_code VARCHAR(20),
    country_code CHAR(2) DEFAULT 'US',

    -- Geolocation (simple lat/lng)
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),
    geocode_accuracy VARCHAR(20),

    -- Contact
    primary_phone VARCHAR(30),
    secondary_phone VARCHAR(30),
    email VARCHAR(255),
    website VARCHAR(500),

    -- Social presence
    linkedin_url VARCHAR(500),
    facebook_url VARCHAR(500),

    -- Status tracking
    status prospect_status DEFAULT 'new',
    status_changed_at TIMESTAMP WITH TIME ZONE,

    -- Source tracking
    data_source VARCHAR(100),
    source_record_id VARCHAR(200),

    -- Flags
    is_verified BOOLEAN DEFAULT false,
    is_duplicate BOOLEAN DEFAULT false,
    duplicate_of_id UUID REFERENCES prospects(id),
    do_not_contact BOOLEAN DEFAULT false,

    -- AI Scoring and Analysis fields (for My Prospects display)
    ai_score INTEGER CHECK (ai_score BETWEEN 0 AND 100),           -- Original AI-derived score
    optimized_score INTEGER CHECK (optimized_score BETWEEN 0 AND 100),  -- Score after scoring rules applied
    relevance_score DECIMAL(5, 4) CHECK (relevance_score BETWEEN 0 AND 1),  -- AI relevance score (0.0-1.0)
    ai_summary TEXT,                                               -- AI-generated summary text
    key_highlights TEXT,                                           -- Pipe-separated list of highlights
    recommended_actions TEXT,                                       -- Pipe-separated list of actions
    data_sources TEXT,                                              -- Comma-separated list of data sources

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospects_territory ON prospects(territory_id);
CREATE INDEX idx_prospects_franchisee ON prospects(franchisee_id);
CREATE INDEX idx_prospects_status ON prospects(status);
CREATE INDEX idx_prospects_industry ON prospects(industry_id);
CREATE INDEX idx_prospects_postal_code ON prospects(postal_code);
CREATE INDEX idx_prospects_created ON prospects(created_at DESC);
CREATE INDEX idx_prospects_business_name ON prospects USING gin(to_tsvector('english', business_name));
CREATE INDEX idx_prospects_optimized_score ON prospects(optimized_score DESC) WHERE optimized_score IS NOT NULL;

COMMENT ON TABLE prospects IS 'Prospective catering clients with full business details';

-- ---------------------------------------------------------------------------
-- Prospect Scores: AI and manual lead scoring
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_scores (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,

    -- Overall score
    total_score INTEGER NOT NULL CHECK (total_score BETWEEN 0 AND 100),
    score_grade CHAR(1) CHECK (score_grade IN ('A', 'B', 'C', 'D', 'F')),

    -- Component scores (0-100 scale)
    fit_score INTEGER CHECK (fit_score BETWEEN 0 AND 100),
    engagement_score INTEGER CHECK (engagement_score BETWEEN 0 AND 100),
    intent_score INTEGER CHECK (intent_score BETWEEN 0 AND 100),

    -- Catering-specific scores
    catering_potential_score INTEGER CHECK (catering_potential_score BETWEEN 0 AND 100),
    order_frequency_score INTEGER CHECK (order_frequency_score BETWEEN 0 AND 100),
    budget_score INTEGER CHECK (budget_score BETWEEN 0 AND 100),

    -- Location scores
    proximity_score INTEGER CHECK (proximity_score BETWEEN 0 AND 100),
    market_density_score INTEGER CHECK (market_density_score BETWEEN 0 AND 100),

    -- Scoring metadata
    score_source score_source NOT NULL,
    model_version VARCHAR(50),
    confidence DECIMAL(5, 4),  -- 0.0000 to 1.0000

    -- Score factors (JSONB for flexibility)
    score_factors JSONB,

    scored_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_scores_prospect ON prospect_scores(prospect_id);
CREATE INDEX idx_prospect_scores_total ON prospect_scores(total_score DESC);
CREATE INDEX idx_prospect_scores_grade ON prospect_scores(score_grade);
CREATE INDEX idx_prospect_scores_scored_at ON prospect_scores(scored_at DESC);

COMMENT ON TABLE prospect_scores IS 'Lead scores from AI models and manual assessment';

-- ---------------------------------------------------------------------------
-- Prospect Contacts: Contact people at prospect businesses
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_contacts (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,

    -- Name
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    full_name VARCHAR(200),

    -- Role
    title VARCHAR(150),
    department VARCHAR(100),
    is_decision_maker BOOLEAN DEFAULT false,
    is_primary_contact BOOLEAN DEFAULT false,

    -- Contact info
    email VARCHAR(255),
    phone VARCHAR(30),
    phone_extension VARCHAR(10),
    mobile VARCHAR(30),
    linkedin_url VARCHAR(500),

    -- Preferences
    preferred_contact_method outreach_type,
    best_time_to_contact VARCHAR(100),

    -- Status
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_contacts_prospect ON prospect_contacts(prospect_id);
CREATE INDEX idx_prospect_contacts_email ON prospect_contacts(email);

COMMENT ON TABLE prospect_contacts IS 'Individual contacts at prospect businesses';

-- ---------------------------------------------------------------------------
-- Prospect Outreach: Track all outreach activities
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_outreach (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,
    contact_id UUID REFERENCES prospect_contacts(id),
    franchisee_id UUID REFERENCES franchisees(id),
    user_id UUID,  -- Sales rep user ID

    -- Outreach details
    outreach_type outreach_type NOT NULL,
    outreach_date TIMESTAMP WITH TIME ZONE NOT NULL,

    -- Content
    subject VARCHAR(300),
    notes TEXT,

    -- Outcome
    was_successful BOOLEAN,
    response_received BOOLEAN DEFAULT false,
    response_date TIMESTAMP WITH TIME ZONE,
    follow_up_required BOOLEAN DEFAULT false,
    follow_up_date DATE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_outreach_prospect ON prospect_outreach(prospect_id);
CREATE INDEX idx_prospect_outreach_date ON prospect_outreach(outreach_date DESC);
CREATE INDEX idx_prospect_outreach_franchisee ON prospect_outreach(franchisee_id);

COMMENT ON TABLE prospect_outreach IS 'History of all outreach activities to prospects';

-- ---------------------------------------------------------------------------
-- Prospect Notes: General notes and activity log
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_notes (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,
    user_id UUID,

    note_type VARCHAR(50) DEFAULT 'general',
    content TEXT NOT NULL,

    is_pinned BOOLEAN DEFAULT false,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_notes_prospect ON prospect_notes(prospect_id);
CREATE INDEX idx_prospect_notes_created ON prospect_notes(created_at DESC);

COMMENT ON TABLE prospect_notes IS 'Notes and activity log for prospects';

-- ---------------------------------------------------------------------------
-- Prospect Tags: Flexible tagging system
-- ---------------------------------------------------------------------------
CREATE TABLE tags (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name VARCHAR(50) NOT NULL UNIQUE,
    color VARCHAR(7),  -- Hex color code
    description VARCHAR(200),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE prospect_tags (
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,
    tag_id UUID NOT NULL REFERENCES tags(id) ON DELETE CASCADE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (prospect_id, tag_id)
);

CREATE INDEX idx_prospect_tags_tag ON prospect_tags(tag_id);

COMMENT ON TABLE tags IS 'Reusable tags for categorizing prospects';
COMMENT ON TABLE prospect_tags IS 'Many-to-many relationship between prospects and tags';

-- ---------------------------------------------------------------------------
-- Saved Searches: Store user search queries
-- ---------------------------------------------------------------------------
CREATE TABLE saved_searches (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID,
    franchisee_id UUID REFERENCES franchisees(id),

    name VARCHAR(100) NOT NULL,
    description TEXT,

    -- Search parameters (stored as JSONB)
    search_params JSONB NOT NULL,

    -- Usage tracking
    last_run_at TIMESTAMP WITH TIME ZONE,
    run_count INTEGER DEFAULT 0,

    is_shared BOOLEAN DEFAULT false,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_saved_searches_franchisee ON saved_searches(franchisee_id);

COMMENT ON TABLE saved_searches IS 'Saved search queries for quick access';

-- ---------------------------------------------------------------------------
-- Data Source Sync: Track data imports from MCP services
-- ---------------------------------------------------------------------------
CREATE TABLE data_source_sync (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    source_name VARCHAR(100) NOT NULL,  -- e.g., 'openstreetmap', 'wikidata'
    sync_type VARCHAR(50) NOT NULL,     -- 'full', 'incremental'

    started_at TIMESTAMP WITH TIME ZONE NOT NULL,
    completed_at TIMESTAMP WITH TIME ZONE,

    records_fetched INTEGER DEFAULT 0,
    records_created INTEGER DEFAULT 0,
    records_updated INTEGER DEFAULT 0,
    records_failed INTEGER DEFAULT 0,

    status VARCHAR(20) DEFAULT 'running',
    error_message TEXT,

    sync_params JSONB,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_data_source_sync_source ON data_source_sync(source_name);
CREATE INDEX idx_data_source_sync_started ON data_source_sync(started_at DESC);

COMMENT ON TABLE data_source_sync IS 'Track data synchronization from MCP data services';


-- ============================================================================
-- VIEWS
-- ============================================================================

-- ---------------------------------------------------------------------------
-- View: Prospects with latest score
-- ---------------------------------------------------------------------------
CREATE VIEW v_prospects_with_scores AS
SELECT
    p.*,
    ps.total_score,
    ps.score_grade,
    ps.fit_score,
    ps.catering_potential_score,
    ps.scored_at,
    ps.score_source,
    t.name as territory_name,
    f.business_name as franchisee_name,
    i.name as industry_name
FROM prospects p
LEFT JOIN LATERAL (
    SELECT * FROM prospect_scores
    WHERE prospect_id = p.id
    ORDER BY scored_at DESC
    LIMIT 1
) ps ON true
LEFT JOIN territories t ON p.territory_id = t.id
LEFT JOIN franchisees f ON p.franchisee_id = f.id
LEFT JOIN industries i ON p.industry_id = i.id;

COMMENT ON VIEW v_prospects_with_scores IS 'Prospects with their latest score and related data';

-- ---------------------------------------------------------------------------
-- View: Territory summary with prospect counts
-- ---------------------------------------------------------------------------
CREATE VIEW v_territory_summary AS
SELECT
    t.id,
    t.name,
    t.code,
    r.name as region_name,
    COUNT(DISTINCT p.id) as total_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'new') as new_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'qualified') as qualified_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'won') as won_prospects,
    COUNT(DISTINCT ft.franchisee_id) as assigned_franchisees,
    td.median_household_income,
    td.total_businesses,
    td.population_density
FROM territories t
LEFT JOIN regions r ON t.region_id = r.id
LEFT JOIN prospects p ON p.territory_id = t.id
LEFT JOIN franchisee_territories ft ON ft.territory_id = t.id
LEFT JOIN LATERAL (
    SELECT * FROM territory_demographics
    WHERE territory_id = t.id
    ORDER BY data_year DESC
    LIMIT 1
) td ON true
GROUP BY t.id, t.name, t.code, r.name,
         td.median_household_income, td.total_businesses, td.population_density;

COMMENT ON VIEW v_territory_summary IS 'Territory overview with prospect counts and demographics';

-- ---------------------------------------------------------------------------
-- View: Franchisee performance dashboard
-- ---------------------------------------------------------------------------
CREATE VIEW v_franchisee_dashboard AS
SELECT
    f.id,
    f.business_name,
    f.franchise_number,
    f.city,
    f.state_province,
    COUNT(DISTINCT p.id) as total_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'new') as new_leads,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status IN ('contacted', 'qualified', 'proposal_sent', 'negotiating')) as active_pipeline,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'won') as won_deals,
    COUNT(DISTINCT po.id) FILTER (WHERE po.outreach_date > CURRENT_DATE - INTERVAL '30 days') as outreach_last_30_days,
    AVG(ps.total_score) as avg_prospect_score,
    COUNT(DISTINCT ft.territory_id) as territory_count
FROM franchisees f
LEFT JOIN prospects p ON p.franchisee_id = f.id
LEFT JOIN prospect_outreach po ON po.franchisee_id = f.id
LEFT JOIN prospect_scores ps ON ps.prospect_id = p.id
LEFT JOIN franchisee_territories ft ON ft.franchisee_id = f.id
WHERE f.is_active = true
GROUP BY f.id, f.business_name, f.franchise_number, f.city, f.state_province;

COMMENT ON VIEW v_franchisee_dashboard IS 'Franchisee performance metrics dashboard';

-- ---------------------------------------------------------------------------
-- View: Hot prospects (high score, recent activity)
-- ---------------------------------------------------------------------------
CREATE VIEW v_hot_prospects AS
SELECT
    p.id,
    p.business_name,
    p.city,
    p.state_province,
    p.status,
    p.employee_count,
    ps.total_score,
    ps.score_grade,
    ps.catering_potential_score,
    t.name as territory_name,
    f.business_name as franchisee_name,
    i.name as industry_name,
    (
        SELECT MAX(outreach_date)
        FROM prospect_outreach
        WHERE prospect_id = p.id
    ) as last_contact_date
FROM prospects p
INNER JOIN LATERAL (
    SELECT * FROM prospect_scores
    WHERE prospect_id = p.id
    ORDER BY scored_at DESC
    LIMIT 1
) ps ON ps.total_score >= 70
LEFT JOIN territories t ON p.territory_id = t.id
LEFT JOIN franchisees f ON p.franchisee_id = f.id
LEFT JOIN industries i ON p.industry_id = i.id
WHERE p.status NOT IN ('won', 'lost', 'inactive')
  AND p.do_not_contact = false
ORDER BY ps.total_score DESC, ps.scored_at DESC;

COMMENT ON VIEW v_hot_prospects IS 'High-scoring prospects ready for outreach';


-- ============================================================================
-- FUNCTIONS
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Function: Update timestamp trigger
-- ---------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Apply to all tables with updated_at
CREATE TRIGGER update_regions_updated_at BEFORE UPDATE ON regions
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_territories_updated_at BEFORE UPDATE ON territories
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_territory_demographics_updated_at BEFORE UPDATE ON territory_demographics
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_franchisees_updated_at BEFORE UPDATE ON franchisees
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_prospects_updated_at BEFORE UPDATE ON prospects
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_prospect_contacts_updated_at BEFORE UPDATE ON prospect_contacts
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_prospect_notes_updated_at BEFORE UPDATE ON prospect_notes
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_saved_searches_updated_at BEFORE UPDATE ON saved_searches
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- ---------------------------------------------------------------------------
-- Function: Calculate prospect score grade from total score
-- ---------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION calculate_score_grade(total_score INTEGER)
RETURNS CHAR(1) AS $$
BEGIN
    RETURN CASE
        WHEN total_score >= 90 THEN 'A'
        WHEN total_score >= 75 THEN 'B'
        WHEN total_score >= 60 THEN 'C'
        WHEN total_score >= 40 THEN 'D'
        ELSE 'F'
    END;
END;
$$ LANGUAGE plpgsql IMMUTABLE;

COMMENT ON FUNCTION calculate_score_grade IS 'Convert numeric score to letter grade';


-- ============================================================================
-- APPLICATION CONFIGURATION TABLE
-- ============================================================================

-- ---------------------------------------------------------------------------
-- App Config: Stores application settings (business/feature settings only)
-- ---------------------------------------------------------------------------
-- NOTE: Infrastructure configs (ApiLogicServer URL, API keys, service endpoints)
-- are stored in local config/app_config.json, NOT in the database.
-- This table stores only business/feature settings managed through the app.

CREATE TABLE app_config (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    config_key VARCHAR(100) NOT NULL UNIQUE,
    config_value TEXT,
    config_type VARCHAR(20) DEFAULT 'string',  -- string, integer, boolean, json

    -- Categorization
    category VARCHAR(50) NOT NULL,  -- 'features', 'display', 'business'

    -- Metadata
    description TEXT,
    is_sensitive BOOLEAN DEFAULT false,
    is_required BOOLEAN DEFAULT false,
    default_value TEXT,

    -- Validation
    validation_regex VARCHAR(500),
    allowed_values TEXT[],  -- For enum-like configs

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_app_config_category ON app_config(category);
CREATE INDEX idx_app_config_key ON app_config(config_key);

CREATE TRIGGER update_app_config_updated_at BEFORE UPDATE ON app_config
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE app_config IS 'Application configuration settings (business/feature settings only)';

-- ---------------------------------------------------------------------------
-- Store Locations: Franchise store locations for the app
-- ---------------------------------------------------------------------------
CREATE TABLE store_locations (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    franchisee_id UUID REFERENCES franchisees(id) ON DELETE CASCADE,

    -- Store identification
    store_name VARCHAR(200) NOT NULL,
    store_code VARCHAR(50) UNIQUE,

    -- Address
    address_line1 VARCHAR(200) NOT NULL,
    address_line2 VARCHAR(100),
    city VARCHAR(100) DEFAULT '',
    state_province VARCHAR(50) DEFAULT '',
    postal_code VARCHAR(20) DEFAULT '',
    country_code CHAR(2) DEFAULT 'US',

    -- Geolocation (simple lat/lng)
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),
    geocode_source VARCHAR(50),  -- 'nominatim', 'google', 'manual'
    geocoded_at TIMESTAMP WITH TIME ZONE,

    -- Search preferences
    default_search_radius_miles DECIMAL(6, 2) DEFAULT 5.0,

    -- Contact
    phone VARCHAR(30),
    email VARCHAR(255),

    -- Status
    is_active BOOLEAN DEFAULT true,
    is_primary BOOLEAN DEFAULT false,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_store_locations_franchisee ON store_locations(franchisee_id);
CREATE INDEX idx_store_locations_active ON store_locations(is_active) WHERE is_active = true;

CREATE TRIGGER update_store_locations_updated_at BEFORE UPDATE ON store_locations
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Migration: Make city/state/postal nullable for existing tables
-- (These fields may not always be available from geocoding)
ALTER TABLE store_locations ALTER COLUMN city SET DEFAULT '';
ALTER TABLE store_locations ALTER COLUMN city DROP NOT NULL;
ALTER TABLE store_locations ALTER COLUMN state_province SET DEFAULT '';
ALTER TABLE store_locations ALTER COLUMN state_province DROP NOT NULL;
ALTER TABLE store_locations ALTER COLUMN postal_code SET DEFAULT '';
ALTER TABLE store_locations ALTER COLUMN postal_code DROP NOT NULL;

COMMENT ON TABLE store_locations IS 'Physical store locations for franchise operations';


-- ============================================================================
-- SCORING RULES TABLE
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Scoring Rules: Configurable rules for prospect score adjustments
-- ---------------------------------------------------------------------------
CREATE TABLE scoring_rules (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    -- Rule identification
    rule_id VARCHAR(50) NOT NULL UNIQUE,       -- e.g., 'no_address', 'bbb_accredited'
    name VARCHAR(100) NOT NULL,                 -- Display name: 'Missing Address'
    description VARCHAR(500),                   -- Explanation of rule

    -- Rule configuration
    is_penalty BOOLEAN DEFAULT false,           -- True for penalties, False for bonuses
    enabled BOOLEAN DEFAULT true,               -- Whether rule is active
    default_points INTEGER NOT NULL,            -- Default point adjustment
    current_points INTEGER NOT NULL,            -- Current configured adjustment
    min_points INTEGER DEFAULT -50,             -- Minimum allowed value
    max_points INTEGER DEFAULT 50,              -- Maximum allowed value

    -- Ownership (for multi-tenant support)
    franchisee_id UUID REFERENCES franchisees(id) ON DELETE CASCADE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

    -- Constraint to ensure points stay in valid range
    CONSTRAINT chk_scoring_rule_points_range CHECK (current_points >= min_points AND current_points <= max_points)
);

CREATE INDEX idx_scoring_rules_rule_id ON scoring_rules(rule_id);
CREATE INDEX idx_scoring_rules_franchisee ON scoring_rules(franchisee_id);
CREATE INDEX idx_scoring_rules_enabled ON scoring_rules(enabled) WHERE enabled = true;

CREATE TRIGGER update_scoring_rules_updated_at BEFORE UPDATE ON scoring_rules
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE scoring_rules IS 'Configurable scoring rules for prospect evaluation with adjustable penalties and bonuses';

-- ---------------------------------------------------------------------------
-- View: Active scoring rules for easy querying
-- ---------------------------------------------------------------------------
CREATE VIEW v_active_scoring_rules AS
SELECT
    rule_id,
    name,
    description,
    is_penalty,
    current_points,
    min_points,
    max_points
FROM scoring_rules
WHERE enabled = true
ORDER BY is_penalty DESC, ABS(current_points) DESC;

COMMENT ON VIEW v_active_scoring_rules IS 'Active scoring rules ordered by type and impact';


-- ============================================================================
-- AUTHENTICATION TABLES
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Users: Application users with authentication
-- ---------------------------------------------------------------------------
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,

    -- Profile
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    phone VARCHAR(30),

    -- Role & Permissions
    role VARCHAR(50) DEFAULT 'franchisee',  -- 'admin', 'franchisee', 'staff'

    -- Franchise Association
    franchisee_id UUID REFERENCES franchisees(id),

    -- Account Status
    is_active BOOLEAN DEFAULT true,
    is_verified BOOLEAN DEFAULT false,
    verification_token VARCHAR(255),

    -- Password Reset
    reset_token VARCHAR(255),
    reset_token_expires TIMESTAMP WITH TIME ZONE,

    -- Session Management
    last_login TIMESTAMP WITH TIME ZONE,
    failed_login_attempts INTEGER DEFAULT 0,
    locked_until TIMESTAMP WITH TIME ZONE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_email_password ON users(email, password_hash);
CREATE INDEX idx_users_franchisee ON users(franchisee_id);

CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE users IS 'Application users with authentication credentials';

-- ---------------------------------------------------------------------------
-- User Sessions: Active login sessions
-- ---------------------------------------------------------------------------
CREATE TABLE user_sessions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,

    session_token VARCHAR(255) NOT NULL UNIQUE,
    refresh_token VARCHAR(255),

    -- Session Info
    ip_address VARCHAR(45),
    user_agent TEXT,

    -- Expiration
    expires_at TIMESTAMP WITH TIME ZONE NOT NULL,
    refresh_expires_at TIMESTAMP WITH TIME ZONE,

    -- Status
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_sessions_token ON user_sessions(session_token);
CREATE INDEX idx_sessions_user ON user_sessions(user_id);

COMMENT ON TABLE user_sessions IS 'Active user login sessions';

-- ---------------------------------------------------------------------------
-- Audit Log: Track security-relevant events
-- ---------------------------------------------------------------------------
CREATE TABLE audit_log (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id),

    event_type VARCHAR(50) NOT NULL,  -- 'login', 'logout', 'failed_login', 'password_change'
    event_details JSONB,

    ip_address VARCHAR(45),
    user_agent TEXT,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_audit_log_user ON audit_log(user_id);
CREATE INDEX idx_audit_log_event ON audit_log(event_type);
CREATE INDEX idx_audit_log_created ON audit_log(created_at DESC);

COMMENT ON TABLE audit_log IS 'Security audit log for authentication events';


-- ============================================================================
-- COMPANY TYPES TABLE
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Company Types: Classification of companies using the FranchiseAI platform
-- ---------------------------------------------------------------------------
CREATE TABLE company_types (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    -- Type identification
    code VARCHAR(50) NOT NULL UNIQUE,           -- e.g., 'franchise', 'manufacturing', 'real_estate'
    name VARCHAR(100) NOT NULL,                  -- Display name
    description TEXT,                            -- Full description

    -- Industry classification
    naics_prefix VARCHAR(4),                     -- NAICS code prefix for this type

    -- Platform configuration
    default_search_radius_miles DECIMAL(6, 2) DEFAULT 10.0,
    default_prospect_industries TEXT[],          -- Suggested industries to target

    -- Status
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_company_types_code ON company_types(code);
CREATE INDEX idx_company_types_active ON company_types(is_active) WHERE is_active = true;

CREATE TRIGGER update_company_types_updated_at BEFORE UPDATE ON company_types
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE company_types IS 'Classification of companies/franchises using the FranchiseAI platform';

-- Add company_type reference to franchisees table
ALTER TABLE franchisees ADD COLUMN IF NOT EXISTS company_type_id UUID REFERENCES company_types(id);
CREATE INDEX IF NOT EXISTS idx_franchisees_company_type ON franchisees(company_type_id);


-- ============================================================================
-- END OF SCHEMA
-- ============================================================================
-- All seed data should be in seed_data.sql
